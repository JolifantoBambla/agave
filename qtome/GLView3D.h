#pragma once

#include <memory>

#include <ome/files/FormatReader.h>

#include "glm.h"
#include "CameraController.h"
#include "GLWindow.h"

#include <QElapsedTimer>

#include "renderlib/Camera.h"

class ImageXYZC;
class QCamera;
class RenderGL;
class RenderGLCuda;
class QTransferFunction;



/**
    * 3D GL view of an image with axes and gridlines.
    */
class GLView3D : public GLWindow
{
    Q_OBJECT

public:
    /// Mouse behaviour.
    enum MouseMode
    {
        MODE_ZOOM,  ///< Zoom in and out.
        MODE_PAN,   ///< Pan in x and y.
        MODE_ROTATE ///< Rotate around point in z.
    };

    /**
    * Create a 2D image view.
    *
    * The size and position will be taken from the specified image.
    *
    * @param reader the image reader.
    * @param series the image series.
    * @param parent the parent of this object.
    */
    GLView3D(std::shared_ptr<ome::files::FormatReader>  reader,
		std::shared_ptr<ImageXYZC>  img,
		ome::files::dimension_size_type                    series,
		QCamera* cam,
		QTransferFunction* tran,
		QWidget                                                *parent = 0);

    /// Destructor.
    ~GLView3D();

    /**
    * Get window minimum size hint.
    *
    * @returns the size hint.
    */
    QSize minimumSizeHint() const;

    /**
    * Get window size hint.
    *
    * @returns the size hint.
    */
    QSize sizeHint() const;

public slots:
    /**
    * Set zoom factor.
    *
    * @param zoom the zoom factor (pixel drag distance).
    */
    void
    setZoom(int zoom);

    /**
    * Set x translation factor.
    *
    * @param xtran x translation factor (pixels).
    */
    void
    setXTranslation(int xtran);

    /**
    * Set y translation factor.
    *
    * @param ytran y translation factor (pixels).
    */
    void
    setYTranslation(int ytran);

    /**
    * Set z rotation factor.
    *
    * @param angle z rotation factor (pixel drag distance).
    */
    void
    setZRotation(int angle);

    /**
    * Set minimum value for linear contrast (all channels).
    *
    * @param min the minimum value (scaled normalized).
    */
    void
    setChannelMin(int min);

    /**
    * Set maximum value for linear contrast (all channels).
    *
    * @param max the maximum value (scaled normalized).
    */
    void
    setChannelMax(int max);

    /**
    * Set plane to render.
    *
    * @param plane the plane number to render.
    */
    void
    setPlane(ome::files::dimension_size_type plane);
	void
	setZCPlane(size_t z, size_t c);

    /**
    * Set mouse behaviour mode.
    *
    * @param mode the behaviour mode to set.
    */
    void
    setMouseMode(MouseMode mode);
	void OnUpdateCamera();
	void OnUpdateTransferFunction(void);

public:
    /**
    * Get reader.
    *
    * @returns the reader.
    */
    std::shared_ptr<ome::files::FormatReader>
    getReader();

    /**
    * Get series.
    *
    * @returns the series.
    */
    ome::files::dimension_size_type
    getSeries();

    /**
    * Get zoom factor.
    *
    * @returns the zoom factor.
    */
    int
    getZoom() const;

    /**
    * Get x translation factor.
    *
    * @returns the x translation factor.
    */
    int
    getXTranslation() const;

    /**
    * Get y translation factor.
    *
    * @returns the y translation factor.
    */
    int
    getYTranslation() const;

    /**
    * Get z rotation factor.
    *
    * @returns the z rotation factor.
    */
    int
    getZRotation() const;

    /**
    * Get minimum value for linear contrast (all channels).
    *
    * @returns the minimum value.
    */
    int
    getChannelMin() const;

    /**
    * Get maximum value for linear contrast (all channels).
    *
    * @returns the maximum value.
    */
    int
    getChannelMax() const;

    /**
    * Get plane to render.
    *
    * @returns the plane number to render.
    */
    ome::files::dimension_size_type
    getPlane() const;
	size_t getZ() const;
	size_t getC() const;


    /**
    * Get mouse behaviour mode.
    *
    * @returns the behaviour mode.
    */
    MouseMode
    getMouseMode() const;

signals:
    /**
    * Signal zoom level changed.
    *
    * @param zoom the new zoom level.
    */
    void
    zoomChanged(int zoom);

    /**
    * Signal x translation changed.
    *
    * @param xtran the new x translation.
    */
    void
    xTranslationChanged(int xtran);

    /**
    * Signal y translation changed.
    *
    * @param ytran the new y translation.
    */
    void
    yTranslationChanged(int ytran);

    /**
    * Signal z rotation changed.
    *
    * @param angle the new z rotation.
    */
    void
    zRotationChanged(int angle);

    /**
    * Signal minimum value for linear contrast changed.
    *
    * @param min the new minimum value.
    */
    void
    channelMinChanged(int min);

    /**
    * Signal maximum value for linear contrast changed.
    *
    * @param max the new maximum value.
    */
    void
    channelMaxChanged(int max);

    /**
    * Signal current plane changed.
    *
    * @param plane the new plane.
    */
    void
    planeChanged(ome::files::dimension_size_type plane);

protected:
    /// Set up GL context and subsidiary objects.
    void
    initialize();

    using GLWindow::render;

    /// Render the scene with the current view settings.
    void
    render();

    /// Resize the view.
    void
    resize();

    /**
    * Handle mouse button press events.
    *
    * Action depends upon the mouse behaviour mode.
    *
    * @param event the event to handle.
    */
    void
    mousePressEvent(QMouseEvent *event);
    void
    mouseReleaseEvent(QMouseEvent *event);

    /**
    * Handle mouse button movement events.
    *
    * Action depends upon the mouse behaviour mode.
    *
    * @param event the event to handle.
    */
    void
    mouseMoveEvent(QMouseEvent *event);

    /**
    * Handle timer events.
    *
    * Used to update scene properties and trigger a render pass.
    *
    * @param event the event to handle.
    */
    void
    timerEvent (QTimerEvent *event);

private:
    CameraController _cameraController;
	QCamera* _camera;
	QTransferFunction* _transferFunction;

    /// Current projection
    Camera camera;
    /// Current mouse behaviour.
    MouseMode mouseMode;
    /// Rendering timer.
    QElapsedTimer etimer;
    /// Minimum level for linear contrast.
    glm::vec3 cmin;
    /// Maximum level for linear contrast.
    glm::vec3 cmax;
    /// Current plane.
    ome::files::dimension_size_type plane;
	size_t _z;
	size_t _c;

    /// Previous plane.
    ome::files::dimension_size_type oldplane;
    /// Last mouse position.
    QPoint lastPos;

	/// The image reader.
	std::shared_ptr<ome::files::FormatReader> reader;
	std::shared_ptr<ImageXYZC> _img;
	/// The image series.
    ome::files::dimension_size_type series;

	std::unique_ptr<RenderGLCuda> _renderGL;
};
