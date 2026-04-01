#pragma once

#include <string>
#include <functional>

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

class Viewer
{
public:
    Viewer(const std::string& title, int width, int height);

    // Main loop – blocks until the user closes the window.
    void run();

    void setMesh(Mesh<2>* mesh);

    // Handle raw mouse messages from the window procedure.
    void handleMouseMessage(unsigned int msg, unsigned long long wParam, long long lParam);

private:
    std::string m_title;
    int m_width;
    int m_height;

    // Scene rotation matrix updated from mouse input.
    Mat4 m_sceneMatrix;

    // Simple track of mouse dragging for rotation.
    bool m_leftDown = false;
    int  m_lastX    = 0;
    int  m_lastY    = 0;
    float m_yaw     = 0.0f;
    float m_pitch   = 0.0f;

    RenderingManager m_render;
};