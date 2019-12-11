/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "gradients.h"
#include "hoverpoints.h"

#include <algorithm>

ShadeWidget::ShadeWidget(ShadeType type, QWidget* parent)
  : QWidget(parent)
  , m_shade_type(type)
  , m_alpha_gradient(QLinearGradient(0, 0, 0, 0))
{

  // Checkers background
  if (m_shade_type == ARGBShade) {
    QPixmap pm(20, 20);
    QPainter pmp(&pm);
    pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
    pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
    pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
    pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
    pmp.end();
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush(pm));
    setAutoFillBackground(true);
    setPalette(pal);

  } else {
    setAttribute(Qt::WA_NoBackground);
  }

  QPolygonF points;
  points << QPointF(0, sizeHint().height()) << QPointF(sizeHint().width(), 0);

  m_hoverPoints = new HoverPoints(this, HoverPoints::CircleShape);
  //     m_hoverPoints->setConnectionType(HoverPoints::LineConnection);
  m_hoverPoints->setPoints(points);
  m_hoverPoints->setPointLock(0, HoverPoints::LockToLeft);
  m_hoverPoints->setPointLock(1, HoverPoints::LockToRight);
  m_hoverPoints->setSortType(HoverPoints::XSort);

  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  connect(m_hoverPoints, &HoverPoints::pointsChanged, this, &ShadeWidget::colorsChanged);
}

QPolygonF
ShadeWidget::points() const
{
  return m_hoverPoints->points();
}

uint
ShadeWidget::colorAt(int x)
{
  generateShade();

  QPolygonF pts = m_hoverPoints->points();
  for (int i = 1; i < pts.size(); ++i) {
    if (pts.at(i - 1).x() <= x && pts.at(i).x() >= x) {
      QLineF l(pts.at(i - 1), pts.at(i));
      l.setLength(l.length() * ((x - l.x1()) / l.dx()));
      return m_shade.pixel(qRound(qMin(l.x2(), (qreal(m_shade.width() - 1)))),
                           qRound(qMin(l.y2(), qreal(m_shade.height() - 1))));
    }
  }
  return 0;
}

void
ShadeWidget::setGradientStops(const QGradientStops& stops)
{
  if (m_shade_type == ARGBShade) {
    m_alpha_gradient = QLinearGradient(0, 0, width(), 0);

    for (int i = 0; i < stops.size(); ++i) {
      QColor c = stops.at(i).second;
      m_alpha_gradient.setColorAt(stops.at(i).first, QColor(c.red(), c.green(), c.blue()));
    }

    m_shade = QImage();
    generateShade();
    update();
  }
}

void
ShadeWidget::paintEvent(QPaintEvent*)
{
  generateShade();

  QPainter p(this);
  p.drawImage(0, 0, m_shade);
  /*
        qreal barWidth = width() / (qreal)m_histogram.size();

      for (int i = 0; i < m_histogram.size(); ++i) {
          qreal h = m_histogram[i] * height();
          // draw level
          painter.fillRect(barWidth * i, height() - h, barWidth * (i + 1), height(), Qt::red);
          // clear the rest of the control
          painter.fillRect(barWidth * i, 0, barWidth * (i + 1), height() - h, Qt::black);
      }
  */
  p.setPen(QColor(146, 146, 146));
  p.drawRect(0, 0, width() - 1, height() - 1);
}

void
ShadeWidget::generateShade()
{
  if (m_shade.isNull() || m_shade.size() != size()) {

    if (m_shade_type == ARGBShade) {
      m_shade = QImage(size(), QImage::Format_ARGB32_Premultiplied);
      m_shade.fill(0);

      QPainter p(&m_shade);
      p.fillRect(rect(), m_alpha_gradient);

      p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
      QLinearGradient fade(0, 0, 0, height()-1);
      fade.setColorAt(0, QColor(0, 0, 0, 255));
      fade.setColorAt(1, QColor(0, 0, 0, 0));
      p.fillRect(rect(), fade);

    } else {
      m_shade = QImage(size(), QImage::Format_RGB32);
      QLinearGradient shade(0, 0, 0, height());
      shade.setColorAt(1, Qt::black);

      if (m_shade_type == RedShade)
        shade.setColorAt(0, Qt::red);
      else if (m_shade_type == GreenShade)
        shade.setColorAt(0, Qt::green);
      else
        shade.setColorAt(0, Qt::blue);

      QPainter p(&m_shade);
      p.fillRect(rect(), shade);
    }
  }
}

GradientEditor::GradientEditor(QWidget* parent)
  : QWidget(parent)
{
  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setSpacing(1);
  vbox->setMargin(1);

  //m_red_shade = new ShadeWidget(ShadeWidget::RedShade, this);
  //m_green_shade = new ShadeWidget(ShadeWidget::GreenShade, this);
  //m_blue_shade = new ShadeWidget(ShadeWidget::BlueShade, this);
  m_alpha_shade = new ShadeWidget(ShadeWidget::ARGBShade, this);

  //  vbox->addWidget(m_red_shade);
  //  vbox->addWidget(m_green_shade);
  //  vbox->addWidget(m_blue_shade);
  vbox->addWidget(m_alpha_shade);

  //  connect(m_red_shade, &ShadeWidget::colorsChanged, this, &GradientEditor::pointsUpdated);
  //  connect(m_green_shade, &ShadeWidget::colorsChanged, this, &GradientEditor::pointsUpdated);
  //  connect(m_blue_shade, &ShadeWidget::colorsChanged, this, &GradientEditor::pointsUpdated);
  connect(m_alpha_shade, &ShadeWidget::colorsChanged, this, &GradientEditor::pointsUpdated);
}

inline static bool
x_less_than(const QPointF& p1, const QPointF& p2)
{
  return p1.x() < p2.x();
}

void
GradientEditor::pointsUpdated()
{
  qreal w = m_alpha_shade->width();

  QGradientStops stops;

  QPolygonF points;

  //points += m_red_shade->points();
  //points += m_green_shade->points();
  //points += m_blue_shade->points();
  points += m_alpha_shade->points();

  std::sort(points.begin(), points.end(), x_less_than);

  for (int i = 0; i < points.size(); ++i) {
    qreal x = int(points.at(i).x());
    if (i + 1 < points.size() && x == points.at(i + 1).x())
      continue;
    unsigned int pixelvalue = m_alpha_shade->colorAt(int(x));
    unsigned int r = (0x00ff0000 & pixelvalue) >> 16;
    unsigned int g = (0x0000ff00 & pixelvalue) >> 8;
    unsigned int b = (0x000000ff & pixelvalue);
    unsigned int a = (0xff000000 & pixelvalue) >> 24;
    QColor color(r,g,b,a);

    if (x / w > 1)
      return;

    stops << QGradientStop(x / w, color);
  }

  m_alpha_shade->setGradientStops(stops);

  emit gradientStopsChanged(stops);
}

static void
set_shade_points(const QPolygonF& points, ShadeWidget* shade)
{
  shade->hoverPoints()->setPoints(points);
  shade->hoverPoints()->setPointLock(0, HoverPoints::LockToLeft);
  shade->hoverPoints()->setPointLock(points.size() - 1, HoverPoints::LockToRight);
  shade->update();
}

void
GradientEditor::setGradientStops(const QGradientStops& stops)
{
  QPolygonF pts_red, pts_green, pts_blue, pts_alpha;

  //qreal h_red = m_red_shade->height();
  //qreal h_green = m_green_shade->height();
  //qreal h_blue = m_blue_shade->height();
  qreal h_alpha = m_alpha_shade->height();

  for (int i = 0; i < stops.size(); ++i) {
    qreal pos = stops.at(i).first;
    QRgb color = stops.at(i).second.rgba();
    //pts_red << QPointF(pos * m_red_shade->width(), h_red - qRed(color) * h_red / 255);
    //pts_green << QPointF(pos * m_green_shade->width(), h_green - qGreen(color) * h_green / 255);
    //pts_blue << QPointF(pos * m_blue_shade->width(), h_blue - qBlue(color) * h_blue / 255);
    pts_alpha << QPointF(pos * m_alpha_shade->width(), h_alpha - qAlpha(color) * h_alpha / 255);
  }

  //set_shade_points(pts_red, m_red_shade);
  //set_shade_points(pts_green, m_green_shade);
  //set_shade_points(pts_blue, m_blue_shade);
  set_shade_points(pts_alpha, m_alpha_shade);
}

GradientWidget::GradientWidget(QWidget* parent)
  : QWidget(parent)
{
  setWindowTitle(tr("Gradients"));

  QGroupBox* mainGroup = new QGroupBox(this);
  mainGroup->setTitle(tr("Gradients"));

  QGroupBox* editorGroup = new QGroupBox(mainGroup);
  editorGroup->setTitle(tr("Color Editor"));
  m_editor = new GradientEditor(editorGroup);

  QGroupBox* presetsGroup = new QGroupBox(mainGroup);
  presetsGroup->setTitle(tr("Presets"));
  QPushButton* prevPresetButton = new QPushButton(tr("<"), presetsGroup);
  m_presetButton = new QPushButton(tr("(unset)"), presetsGroup);
  QPushButton* nextPresetButton = new QPushButton(tr(">"), presetsGroup);
  updatePresetName();

  QGroupBox* defaultsGroup = new QGroupBox(mainGroup);
  defaultsGroup->setTitle(tr("Examples"));
  QPushButton* default1Button = new QPushButton(tr("1"), defaultsGroup);
  QPushButton* default2Button = new QPushButton(tr("2"), defaultsGroup);
  QPushButton* default3Button = new QPushButton(tr("3"), defaultsGroup);
  QPushButton* default4Button = new QPushButton(tr("Reset"), editorGroup);

  // Layouts
  QHBoxLayout* mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(mainGroup);

  mainGroup->setFixedWidth(200);
  QVBoxLayout* mainGroupLayout = new QVBoxLayout(mainGroup);
  mainGroupLayout->addWidget(editorGroup);
  mainGroupLayout->addWidget(presetsGroup);
  mainGroupLayout->addWidget(defaultsGroup);
  mainGroupLayout->addStretch(1);

  QVBoxLayout* editorGroupLayout = new QVBoxLayout(editorGroup);
  editorGroupLayout->addWidget(m_editor);

  QHBoxLayout* presetsGroupLayout = new QHBoxLayout(presetsGroup);
  presetsGroupLayout->addWidget(prevPresetButton);
  presetsGroupLayout->addWidget(m_presetButton, 1);
  presetsGroupLayout->addWidget(nextPresetButton);

  QHBoxLayout* defaultsGroupLayout = new QHBoxLayout(defaultsGroup);
  defaultsGroupLayout->addWidget(default1Button);
  defaultsGroupLayout->addWidget(default2Button);
  defaultsGroupLayout->addWidget(default3Button);
  editorGroupLayout->addWidget(default4Button);

  //  connect(m_editor, &GradientEditor::gradientStopsChanged, m_renderer, &GradientRenderer::setGradientStops);

  connect(prevPresetButton, &QPushButton::clicked, this, &GradientWidget::setPrevPreset);
  connect(m_presetButton, &QPushButton::clicked, this, &GradientWidget::setPreset);
  connect(nextPresetButton, &QPushButton::clicked, this, &GradientWidget::setNextPreset);

  connect(default1Button, &QPushButton::clicked, this, &GradientWidget::setDefault1);
  connect(default2Button, &QPushButton::clicked, this, &GradientWidget::setDefault2);
  connect(default3Button, &QPushButton::clicked, this, &GradientWidget::setDefault3);
  connect(default4Button, &QPushButton::clicked, this, &GradientWidget::setDefault4);

  connect(m_editor, &GradientEditor::gradientStopsChanged, this, &GradientWidget::onGradientStopsChanged);

  QTimer::singleShot(50, this, SLOT(setDefault4()));
}

void
GradientWidget::onGradientStopsChanged(const QGradientStops& stops)
{
  emit gradientStopsChanged(stops);
}

void
GradientWidget::setDefault(int config)
{
  QGradientStops stops;
  QPolygonF points;
  switch (config) {
    case 1:
      stops << QGradientStop(0.00, QColor::fromRgba(0));
      stops << QGradientStop(0.04, QColor::fromRgba(0xff131360));
      stops << QGradientStop(0.08, QColor::fromRgba(0xff202ccc));
      stops << QGradientStop(0.42, QColor::fromRgba(0xff93d3f9));
      stops << QGradientStop(0.51, QColor::fromRgba(0xffb3e6ff));
      stops << QGradientStop(0.73, QColor::fromRgba(0xffffffec));
      stops << QGradientStop(0.92, QColor::fromRgba(0xff5353d9));
      stops << QGradientStop(0.96, QColor::fromRgba(0xff262666));
      stops << QGradientStop(1.00, QColor::fromRgba(0));
      break;

    case 2:
      stops << QGradientStop(0.00, QColor::fromRgba(0xffffffff));
      stops << QGradientStop(0.11, QColor::fromRgba(0xfff9ffa0));
      stops << QGradientStop(0.13, QColor::fromRgba(0xfff9ff99));
      stops << QGradientStop(0.14, QColor::fromRgba(0xfff3ff86));
      stops << QGradientStop(0.49, QColor::fromRgba(0xff93b353));
      stops << QGradientStop(0.87, QColor::fromRgba(0xff264619));
      stops << QGradientStop(0.96, QColor::fromRgba(0xff0c1306));
      stops << QGradientStop(1.00, QColor::fromRgba(0));
      break;

    case 3:
      stops << QGradientStop(0.00, QColor::fromRgba(0));
      stops << QGradientStop(0.10, QColor::fromRgba(0xffe0cc73));
      stops << QGradientStop(0.17, QColor::fromRgba(0xffc6a006));
      stops << QGradientStop(0.46, QColor::fromRgba(0xff600659));
      stops << QGradientStop(0.72, QColor::fromRgba(0xff0680ac));
      stops << QGradientStop(0.92, QColor::fromRgba(0xffb9d9e6));
      stops << QGradientStop(1.00, QColor::fromRgba(0));
      break;

    case 4:
      stops << QGradientStop(0.00, QColor::fromRgba(0xff000000));
      stops << QGradientStop(1.00, QColor::fromRgba(0xffffffff));
      break;

    default:
      qWarning("bad default: %d\n", config);
      break;
  }

  m_editor->setGradientStops(stops);
}

void
GradientWidget::updatePresetName()
{
  QMetaEnum presetEnum = QMetaEnum::fromType<QGradient::Preset>();
  m_presetButton->setText(QLatin1String(presetEnum.key(m_presetIndex)));
}

void
GradientWidget::changePresetBy(int indexOffset)
{
  QMetaEnum presetEnum = QMetaEnum::fromType<QGradient::Preset>();
  m_presetIndex = qBound(0, m_presetIndex + indexOffset, presetEnum.keyCount() - 1);

  QGradient::Preset preset = static_cast<QGradient::Preset>(presetEnum.value(m_presetIndex));
  QGradient gradient(preset);
  if (gradient.type() != QGradient::LinearGradient)
    return;

  m_editor->setGradientStops(gradient.stops());

  updatePresetName();
}
