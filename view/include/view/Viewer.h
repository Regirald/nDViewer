#pragma once

#include <string>

// Minimal viewer interface to open a window and run the main loop.
// The implementation will live in view/src/blabla.cpp.
class Viewer
{
public:
    Viewer(const std::string& title, int width, int height);

    // Main loop – blocks until the user closes the window.
    void run();

    // For now this is a no-op placeholder; you can hook your mesh/vision data here.
    template <typename MeshT>
    void setMesh(const MeshT&)
    {
        // Intentionally empty – connect to your rendering later.
    }

private:
    std::string m_title;
    int m_width;
    int m_height;
};