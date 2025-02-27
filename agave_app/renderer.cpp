#include "renderer.h"

#include "renderlib/CCamera.h"
#include "renderlib/FileReader.h"
#include "renderlib/Logging.h"
#include "renderlib/RenderGLPT.h"
#include "renderlib/RenderSettings.h"

#include "command.h"
#include "commandBuffer.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QOpenGLFramebufferObjectFormat>

Renderer::Renderer(QString id, QObject* parent, QMutex& mutex)
  : QThread(parent)
  , m_id(id)
  , m_streamMode(0)
  , m_fbo(nullptr)
  , m_width(0)
  , m_height(0)
  , m_openGLMutex(&mutex)
{
  this->m_totalQueueDuration = 0;

  LOG_DEBUG << "Renderer " << id.toStdString() << " -- Initializing rendering thread...";
  LOG_DEBUG << "Renderer " << id.toStdString() << " -- Done.";
}

Renderer::~Renderer()
{
  // delete all outstanding requests.
  qDeleteAll(this->m_requests);
}

void
Renderer::myVolumeInit()
{
  static const int initWidth = 1024, initHeight = 1024;

  m_myVolumeData.m_renderSettings = new RenderSettings();

  m_myVolumeData.m_camera = new CCamera();
  m_myVolumeData.m_camera->m_Film.m_ExposureIterations = 1;
  m_myVolumeData.m_camera->m_Film.m_Resolution.SetResX(initWidth);
  m_myVolumeData.m_camera->m_Film.m_Resolution.SetResY(initHeight);

  m_myVolumeData.m_scene = new Scene();
  m_myVolumeData.m_scene->initLights();

  m_myVolumeData.m_renderer = new RenderGLPT(m_myVolumeData.m_renderSettings);
  m_myVolumeData.m_renderer->initialize(initWidth, initHeight);
  m_myVolumeData.m_renderer->setScene(m_myVolumeData.m_scene);
}

void
Renderer::init()
{
  // this->setFixedSize(1920, 1080);
  // QMessageBox::information(this, "Info:", "Application Directory: " + QApplication::applicationDirPath() + "\n" +
  // "Working Directory: " + QDir::currentPath());

#if HAS_EGL
  this->m_glContext = new HeadlessGLContext();
  this->m_glContext->makeCurrent();
#else
  this->m_glContext = renderlib::createOpenGLContext();

  this->m_surface = new QOffscreenSurface();
  this->m_surface->setFormat(this->m_glContext->format());
  this->m_surface->create();

  /*this->context->doneCurrent();
  this->context->moveToThread(this);*/
  this->m_glContext->makeCurrent(m_surface);
#endif

  int status = gladLoadGL();
  if (!status) {
    LOG_INFO << m_id.toStdString() << " COULD NOT LOAD GL ON THREAD";
  }

  ///////////////////////////////////
  // INIT THE RENDER LIB
  ///////////////////////////////////

  this->resizeGL(1024, 1024);

  int MaxSamples = 0;
  glGetIntegerv(GL_MAX_SAMPLES, &MaxSamples);
  LOG_INFO << m_id.toStdString() << " max samples" << MaxSamples;

  glEnable(GL_MULTISAMPLE);

  reset();

  this->m_glContext->doneCurrent();
#if HAS_EGL
#else
  this->m_glContext->moveToThread(this);
#endif
}

void
Renderer::run()
{
  this->init();

#if HAS_EGL
  this->m_glContext->makeCurrent();
#else
  this->m_glContext->makeCurrent(this->m_surface);
#endif

  // TODO: PUT THIS KIND OF INIT SOMEWHERE ELSE
  myVolumeInit();

  while (!QThread::currentThread()->isInterruptionRequested()) {
    this->processRequest();

    QApplication::processEvents();
  }

#if HAS_EGL
  this->m_glContext->makeCurrent();
#else
  this->m_glContext->makeCurrent(this->m_surface);
#endif
  m_myVolumeData.m_renderer->cleanUpResources();
  shutDown();
}

void
Renderer::addRequest(RenderRequest* request)
{
  this->m_requests << request;
  this->m_totalQueueDuration += request->getDuration();
}

bool
Renderer::processRequest()
{
  if (this->m_requests.isEmpty()) {
    return false;
  }

  if (m_streamMode != 0) {
    QElapsedTimer timer;
    timer.start();

    RenderRequest* lastReq = nullptr;

    // eat requests until done, and then render
    // note that any one request could change the streaming mode.
    while (!this->m_requests.isEmpty() && m_streamMode != 0) {

      RenderRequest* r = this->m_requests.takeFirst();
      this->m_totalQueueDuration -= r->getDuration();

      std::vector<Command*> cmds = r->getParameters();
      if (cmds.size() > 0) {
        this->processCommandBuffer(r);
      }

      // the true last request will be passed to "emit" and deleted later
      if (!this->m_requests.isEmpty() && m_streamMode != 0) {
        delete r;
      } else {
        lastReq = r;
      }
    }

    QWebSocket* ws = lastReq->getClient();
    LOG_DEBUG << "RENDER for " << ws->peerName().toStdString() << "(" << ws->peerAddress().toString().toStdString()
              << ":" << QString::number(ws->peerPort()).toStdString() << ")";

    QImage img = this->render();

    lastReq->setActualDuration(timer.nsecsElapsed());

    // in stream mode:
    // if queue is empty, then keep firing redraws back to client.
    // test about 100 frames as a convergence limit.
    if (m_streamMode != 0 && m_myVolumeData.m_renderSettings->GetNoIterations() < 500) {
      // push another redraw request.
      std::vector<Command*> cmd;
      RequestRedrawCommandD data;
      cmd.push_back(new RequestRedrawCommand(data));
      this->addRequest(new RenderRequest(lastReq->getClient(), cmd, false));
    }

    // inform the server that we are done with r
    emit requestProcessed(lastReq, img);

  } else {
    // if not in stream mode, then process one request, then re-render.

    // remove request from the queue
    RenderRequest* r = this->m_requests.takeFirst();
    this->m_totalQueueDuration -= r->getDuration();

    // process it
    QElapsedTimer timer;
    timer.start();

    std::vector<Command*> cmds = r->getParameters();
    if (cmds.size() > 0) {
      this->processCommandBuffer(r);
    }

    QImage img = this->render();

    r->setActualDuration(timer.nsecsElapsed());

    // inform the server that we are done with r
    emit requestProcessed(r, img);
  }

  return true;
}

void
Renderer::processCommandBuffer(RenderRequest* rr)
{
#if HAS_EGL
  this->m_glContext->makeCurrent();
#else
  this->m_glContext->makeCurrent(this->m_surface);
#endif

  std::vector<Command*> cmds = rr->getParameters();
  if (cmds.size() > 0) {
    m_ec.m_renderSettings = m_myVolumeData.m_renderSettings;
    m_ec.m_renderer = this;
    m_ec.m_appScene = m_myVolumeData.m_scene;
    m_ec.m_camera = m_myVolumeData.m_camera;
    m_ec.m_message = "";

    for (auto i = cmds.begin(); i != cmds.end(); ++i) {
      (*i)->execute(&m_ec);
      if (!m_ec.m_message.empty()) {
        emit sendString(rr, QString::fromStdString(m_ec.m_message));
        m_ec.m_message = "";
      }
    }
  }
}

QImage
Renderer::render()
{
  m_openGLMutex->lock();
#if HAS_EGL
  this->m_glContext->makeCurrent();
#else
  this->m_glContext->makeCurrent(this->m_surface);
#endif

  // DRAW
  m_myVolumeData.m_camera->Update();
  m_myVolumeData.m_renderer->doRender(*(m_myVolumeData.m_camera));

  // COPY TO MY FBO
  this->m_fbo->bind();
  glViewport(0, 0, m_fbo->width(), m_fbo->height());
  m_myVolumeData.m_renderer->drawImage();
  this->m_fbo->release();

  std::unique_ptr<uint8_t> bytes(new uint8_t[m_fbo->width() * m_fbo->height() * 4]);
  m_fbo->toImage(bytes.get());
  QImage img = QImage(bytes.get(), m_fbo->width(), m_fbo->height(), QImage::Format_RGB32).copy().mirrored();

  this->m_glContext->doneCurrent();
  m_openGLMutex->unlock();

  return img;
}

void
Renderer::resizeGL(int width, int height)
{
  if ((width == m_width) && (height == m_height)) {
    return;
  }
  m_openGLMutex->lock();

#if HAS_EGL
  this->m_glContext->makeCurrent();
#else
  this->m_glContext->makeCurrent(this->m_surface);
#endif
  // RESIZE THE RENDER INTERFACE
  if (m_myVolumeData.m_renderer) {
    m_myVolumeData.m_renderer->resize(width, height);
  }

  delete this->m_fbo;
  this->m_fbo = new GLFramebufferObject(width, height, GL_RGBA8);

  glViewport(0, 0, width, height);

  m_width = width;
  m_height = height;

  m_openGLMutex->unlock();
}

void
Renderer::reset(int from)
{
  m_openGLMutex->lock();

#if HAS_EGL
  this->m_glContext->makeCurrent();
#else
  this->m_glContext->makeCurrent(this->m_surface);
#endif

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);

  this->m_time.start();

  m_openGLMutex->unlock();
}

int
Renderer::getTime()
{
  return this->m_time.elapsed();
}

void
Renderer::shutDown()
{
#if HAS_EGL
  this->m_glContext->makeCurrent();
#else
  this->m_glContext->makeCurrent(this->m_surface);
#endif
  delete this->m_fbo;

  delete m_myVolumeData.m_renderSettings;
  delete m_myVolumeData.m_camera;
  delete m_myVolumeData.m_scene;
  delete m_myVolumeData.m_renderer;
  m_myVolumeData.m_camera = nullptr;
  m_myVolumeData.m_scene = nullptr;
  m_myVolumeData.m_renderSettings = nullptr;
  m_myVolumeData.m_renderer = nullptr;

  m_glContext->doneCurrent();
  delete m_glContext;

#if HAS_EGL
#else
  // schedule this to be deleted only after we're done cleaning up
  m_surface->deleteLater();
#endif

  // Stop event processing, move the thread to GUI and make sure it is deleted.
  exit();
  moveToThread(QGuiApplication::instance()->thread());
}
