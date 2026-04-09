#pragma once

#include <string>

#include <QOpenGLWidget>
#include <QTimer>
#include <QKeyEvent>

#include "Camera/Camera.h"
#include "Renderer/RenderingManager.h"

class Viewer : public QOpenGLWidget
{
public:
    explicit Viewer(QWidget* parent = nullptr);

    void setMesh(Mesh<2>* mesh);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void updateCameraFromOrbit();
    void setYawPitchFromInitialPosition();

    // Simple track of mouse dragging for rotation.
    ndv::Camera m_camera;

    bool m_leftDown = false;
    bool m_rightDown = false;
    bool m_middleDown = false;
    int  m_lastX    = 0;
    int  m_lastY    = 0;
    float m_yaw     = 0.0f;
    float m_pitch   = 0.0f;
    float m_zoom    = 1.0f;
    float m_baseDistance = 3.0f;


    RenderingManager renderingManager;

    QTimer m_updateTimer;

    // Cached frames-per-second counter for on-screen display.
    float m_fps = 0.0f;
};