#include "RenderBackground.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    GLuint EnsureFontListBase()
    {
        static GLuint listBase = 0;
        static bool initialized = false;
        if (initialized)
        {
            return listBase;
        }

        initialized = true;
        HDC hdc = wglGetCurrentDC();
        if (!hdc)
        {
            return 0;
        }

        listBase = glGenLists(96);
        HFONT font = CreateFontA(
            -14,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            ANSI_CHARSET,
            OUT_TT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            "Consolas");

        HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));
        wglUseFontBitmapsA(hdc, 32, 96, listBase);
        SelectObject(hdc, oldFont);
        DeleteObject(font);

        return listBase;
    }

    void DrawText2D(float x, float y, const std::string& text)
    {
        const GLuint listBase = EnsureFontListBase();
        if (!listBase || text.empty())
        {
            return;
        }

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(x, y);
        glPushAttrib(GL_LIST_BIT);
        glListBase(listBase - 32);
        glCallLists(static_cast<GLsizei>(text.size()), GL_UNSIGNED_BYTE, text.c_str());
        glPopAttrib();
    }
}

void RenderBackground::render()
{
    static auto lastTime = std::chrono::steady_clock::now();
    static int frameCount = 0;
    static float fps = 0.0f;

    ++frameCount;
    const auto now = std::chrono::steady_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();
    if (elapsedMs >= 250)
    {
        fps = (static_cast<float>(frameCount) * 1000.0f) / static_cast<float>(elapsedMs);
        frameCount = 0;
        lastTime = now;
    }

    GLfloat modelView[16] = {};
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

    GLint viewport[4] = {};
    glGetIntegerv(GL_VIEWPORT, viewport);
    const int width = viewport[2];
    const int height = viewport[3];
    if (width <= 0 || height <= 0)
    {
        return;
    }

    const float axisX = std::fabs(modelView[2]);
    const float axisY = std::fabs(modelView[6]);
    const float axisZ = std::fabs(modelView[10]);

    const float base = 0.1f;
    const float scale = 0.8f;

    std::vector<GLubyte> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 3u);
    for (int y = 0; y < height; ++y)
    {
        const float ny = static_cast<float>(y) / static_cast<float>(height - 1);
        for (int x = 0; x < width; ++x)
        {
            const float nx = static_cast<float>(x) / static_cast<float>(width - 1);
            const float r = std::clamp(base + scale * (axisX * nx + (1.0f - axisX) * ny), 0.0f, 1.0f);
            const float g = std::clamp(base + scale * (axisY * ny + (1.0f - axisY) * nx), 0.0f, 1.0f);
            const float b = std::clamp(base + scale * (axisZ * (1.0f - nx) + (1.0f - axisZ) * (1.0f - ny)), 0.0f, 1.0f);

            const size_t index = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 3u;
            pixels[index] = static_cast<GLubyte>(r * 255.0f);
            pixels[index + 1] = static_cast<GLubyte>(g * 255.0f);
            pixels[index + 2] = static_cast<GLubyte>(b * 255.0f);
        }
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2f(-1.0f, -1.0f);
    glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    std::ostringstream fpsText;
    fpsText << "FPS: " << std::fixed << std::setprecision(1) << fps;
    DrawText2D(-0.95f, 0.9f, fpsText.str());
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}
