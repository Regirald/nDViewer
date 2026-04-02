#include <QMouseEvent>

#include "Viewer.h"

Viewer::Viewer(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    m_render.setBackground();
    // regular update of the viewer
    m_updateTimer.setInterval(16);
    connect(&m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&Viewer::update));
    m_updateTimer.start();
}

void Viewer::setMesh(Mesh<2>* mesh)
{
    m_render.setMesh(mesh);
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
}

void Viewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m_sceneMatrix.m);
    glScalef(m_zoom, m_zoom, m_zoom);

    m_render.renderScene();

    glBegin(GL_QUADS);

    // +X face (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);

    // -X face (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);

    // +Y face (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);

    // -Y face (yellow)
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);

    // +Z face (cyan)
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);

    // -Z face (magenta)
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);

    glEnd();
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

        const float degToRad = 3.1415926535f / 180.0f;
        const float cy = cosf(m_yaw * degToRad);
        const float sy = sinf(m_yaw * degToRad);
        const float cp = cosf(m_pitch * degToRad);
        const float sp = sinf(m_pitch * degToRad);

        Mat4 rot;
        rot.m[0] = cy;      rot.m[1] = 0.0f;  rot.m[2]  = sy;      rot.m[3]  = 0.0f;
        rot.m[4] = sy*sp;   rot.m[5] = cp;    rot.m[6]  = -cy*sp;  rot.m[7]  = 0.0f;
        rot.m[8] = -sy*cp;  rot.m[9] = sp;    rot.m[10] = cy*cp;   rot.m[11] = 0.0f;
        rot.m[12] = 0.0f;   rot.m[13] = 0.0f; rot.m[14] = 0.0f;    rot.m[15] = 1.0f;

        m_sceneMatrix = rot;
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

    update();
}
