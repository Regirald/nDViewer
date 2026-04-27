#include <QElapsedTimer>
#include <QMouseEvent>
#include <QClipboard>
#include <QOpenGLFramebufferObject>

#include <algorithm>

#include "Viewer.h"
#include "fileInfo.h"

Viewer::Viewer(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    renderingManager.setBackground();
    //renderingManager.setExampleTriangle();
    setYawPitchFromInitialPosition();
    updateCameraFromOrbit();
    // regular update of the viewer
    m_updateTimer.setInterval(16);
    connect(&m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&Viewer::update));
    m_updateTimer.start();
}

void Viewer::initializeGL()
{
    glClearColor(0.f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void Viewer::resizeGL(int w, int h)
{
    if (h <= 0)
        h = 1;

    glViewport(0, 0, w, h);
    renderingManager.setViewport(w, h);
    m_camera.aspectRatio = static_cast<float>(w) / static_cast<float>(h);
}

void Viewer::paintGL()
{
    static QElapsedTimer timer;
    if (!timer.isValid())
    {
        timer.start();
    }

    const int viewportWidth = width();
    const int viewportHeight = height() > 0 ? height() : 1;
    renderingManager.setViewport(viewportWidth, viewportHeight);
    m_camera.aspectRatio = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);

    renderingManager.setCamera(m_camera.viewMatrix(), m_camera.projectionMatrix(), m_camera.position, m_camera.target);
    renderingManager.setTime(static_cast<float>(timer.elapsed()) / 1000.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderingManager.renderScene();
    qDebug() << "CTX:" << QOpenGLContext::currentContext();

}

void Viewer::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_C) 
        {
            // copy to clipboard
            QImage screenshot = grabFramebuffer();
            QGuiApplication::clipboard()->setImage(screenshot);
        }
        else if (event->key() == Qt::Key_S)
        { 
            captureHighRes(); // 2x supersampling
        }
        return; // consume event
    }

    QOpenGLWidget::keyPressEvent(event);
}

void Viewer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_leftDown = true;
        m_lastX = static_cast<int>(event->position().x());
        m_lastY = static_cast<int>(event->position().y());
    }
    else if (event->button() == Qt::RightButton)
    {
        m_rightDown = true;
        m_lastX = static_cast<int>(event->position().x());
        m_lastY = static_cast<int>(event->position().y());
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_middleDown = true;
        m_lastX = static_cast<int>(event->position().x());
        m_lastY = static_cast<int>(event->position().y());
    }
}

void Viewer::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_leftDown = false;
    }
    else if (event->button() == Qt::RightButton)
    {
        m_rightDown = false;
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_middleDown = false;
    }
}

void Viewer::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_leftDown && !m_rightDown && !m_middleDown)
        return;

    const int x = static_cast<int>(event->position().x());
    const int y = static_cast<int>(event->position().y());

    const int dx = x - m_lastX;
    const int dy = y - m_lastY;

    m_lastX = x;
    m_lastY = y;

    if (m_leftDown)
    {
        const float sensitivity = 0.25f;
        m_yaw   += dx * sensitivity;
        m_pitch += dy * sensitivity;
    }
    else if (m_rightDown)
    {
        const float zoomSensitivity = 0.01f;
        m_zoom += -dy * zoomSensitivity;
        if (m_zoom < 0.1f)
            m_zoom = 0.1f;
        if (m_zoom > 10.0f)
            m_zoom = 10.0f;
    }
    else if (m_middleDown)
    {
        // pan in the plane parallel to the screen (UV plane)
        const float panSpeed = 0.01f;

        // view direction
        QVector3D forward = (m_camera.target - m_camera.position).normalized();
        // derive camera up in view space
        QVector3D up = m_camera.up;
        // right = forward x up
        QVector3D right = QVector3D::crossProduct(forward, up).normalized();
        QVector3D newUp = QVector3D::crossProduct(forward, right).normalized();
        up = newUp;

        // move opposite to mouse dx/dy to feel natural
        QVector3D pan =
            right * (-dx * panSpeed) +
            up * (-dy * panSpeed);

        m_camera.target   += pan;
    }

    updateCameraFromOrbit();
    update();
}

void Viewer::updateCameraFromOrbit()
{
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    const float degToRad = 3.1415926535f / 180.0f;
    const float yawRad = m_yaw * degToRad;
    const float pitchRad = m_pitch * degToRad;

    const float distance = m_baseDistance / m_zoom;
    const float cosPitch = cosf(pitchRad);
    QVector3D offset(
        distance * cosPitch * sinf(yawRad),
        distance * sinf(pitchRad),
        distance * cosPitch * cosf(yawRad));



    //m_camera.target = { 0.0f, 0.0f, 0.0f };
    //m_camera.up = QVector3D(0.0f, 1.0f, 0.0f);
    m_camera.position = m_camera.target + offset;
}

void Viewer::setYawPitchFromInitialPosition()
{
    QVector3D dir = m_camera.position - m_camera.target;

    m_baseDistance = dir.length();
    if (m_baseDistance < 1e-6f)
        return; // avoid division by zero

    // normalize direction
    QVector3D n = dir / m_baseDistance;

    const float radToDeg = 180.0f / 3.1415926535f;

    // pitch = asin(y)
    m_pitch = asinf(n.y()) * radToDeg;

    // yaw = atan2(x, z)
    m_yaw = atan2f(n.x(), n.z()) * radToDeg;

    // optional: keep pitch in your expected range
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void Viewer::captureHighRes()
{
    makeCurrent();  // CRITICAL
    int width = 1920;
    int height = 1080;

    // Create FBO
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    QOpenGLFramebufferObject fbo(width, height, format);

    // Bind FBO
    fbo.bind(); 

    // Save current viewport
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    // Set new viewport
    glViewport(0, 0, width, height);

    // Update camera aspect ratio
    float oldAspect = m_camera.aspectRatio;
    m_camera.aspectRatio = float(width) / float(height);

    // Render scene (same as paintGL, but without swapbuffers)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    renderingManager.setViewport(width, height);
    renderingManager.setCamera(
        m_camera.viewMatrix(),
        m_camera.projectionMatrix(),
        m_camera.position,
        m_camera.target
    );

    renderingManager.renderScene();

    // Restore state
    m_camera.aspectRatio = oldAspect;
    glViewport(prevViewport[0], prevViewport[1],
        prevViewport[2], prevViewport[3]);

    fbo.release();

    // Read image
    QImage image = fbo.toImage();


    // Save
    QString filename = QString("%1snapshot.png").arg(getSnapShotDirectory());
    filename = getAvailableFilename(filename);

    image.save(filename);
}