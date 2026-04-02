#pragma once

#include <string>

#include <QOpenGLWidget>
#include <QTimer>

#include "RenderingManager.h"


// Simple 4x4 float matrix for scene transforms.
struct Mat4
{
    float m[16] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
};

class Viewer : public QOpenGLWidget
{
public:
    explicit Viewer(QWidget* parent = nullptr);

    void setMesh(Mesh<2>* mesh);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    // Scene rotation matrix updated from mouse input.
    Mat4 m_sceneMatrix;

    // Simple track of mouse dragging for rotation.
    bool m_leftDown = false;
    bool m_rightDown = false;
    int  m_lastX    = 0;
    int  m_lastY    = 0;
    float m_yaw     = 0.0f;
    float m_pitch   = 0.0f;
    float m_zoom    = 1.0f;

    RenderingManager m_render;

    QTimer m_updateTimer;

    // Cached frames-per-second counter for on-screen display.
    float m_fps = 0.0f;
};