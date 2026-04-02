#include <QElapsedTimer>
#include <QMouseEvent>

#include <algorithm>

#include "Viewer.h"

Viewer::Viewer(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    renderingManager.setBackground();
    updateCameraFromOrbit();
    // regular update of the viewer
    m_updateTimer.setInterval(16);
    connect(&m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&Viewer::update));
    m_updateTimer.start();
}

void Viewer::setMesh(Mesh<2>* mesh)
{
    renderingManager.setMesh(mesh);
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

    renderingManager.setCamera(m_camera.viewMatrix(), m_camera.projectionMatrix(), m_camera.position);
    renderingManager.setTime(static_cast<float>(timer.elapsed()) / 1000.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderingManager.renderScene();

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
}

void Viewer::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_leftDown && !m_rightDown)
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
    ndv::math::Vec3 offset;
    offset.x = distance * cosPitch * sinf(yawRad);
    offset.y = distance * sinf(pitchRad);
    offset.z = distance * cosPitch * cosf(yawRad);



    m_camera.target = { 0.0f, 0.0f, 0.0f };
    m_camera.up = { 0.0f, 1.0f, 0.0f };
    m_camera.position = m_camera.target + offset;
}
