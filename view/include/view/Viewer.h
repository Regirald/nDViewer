#pragma once

#include <string>
#include <functional>

class Viewer
{
public:
    Viewer(const std::string& title, int width, int height);

    // Main loop – blocks until the user closes the window.
    void run();

    // Set a render callback that is called every frame.
    // The callback receives the current window width/height.
    using RenderCallback = std::function<void(int width, int height)>;
    void setRenderCallback(RenderCallback cb) { m_renderCallback = std::move(cb); }

private:
    std::string m_title;
    int m_width;
    int m_height;

    RenderCallback m_renderCallback;
};