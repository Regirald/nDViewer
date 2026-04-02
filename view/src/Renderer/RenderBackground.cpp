#include "RenderBackground.h"

#include <Windows.h>
#include <gl/GL.h>

#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include <QOpenGLShaderProgram>
#include <QVector2D>
#include <QVector3D>

namespace
{
    QOpenGLShaderProgram& BackgroundProgram()
    {
        static QOpenGLShaderProgram program;
        static bool initialized = false;
        if (initialized)
        {
            return program;
        }

        initialized = true;
        const char* vertexSource =
            "#version 120\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(gl_Vertex.xy, 0.0, 1.0);\n"
            "}\n";

        const char* fragmentSource =
            "#version 120\n"
            "uniform vec3 u_axis;\n"
            "uniform vec2 u_resolution;\n"
            "uniform float u_base;\n"
            "uniform float u_scale;\n"
            "void main()\n"
            "{\n"
            "    vec2 uv = gl_FragCoord.xy / u_resolution;\n"
            "    float r = clamp(u_base + u_scale * (u_axis.x * uv.x + (1.0 - u_axis.x) * uv.y), 0.0, 1.0);\n"
            "    float g = clamp(u_base + u_scale * (u_axis.y * uv.y + (1.0 - u_axis.y) * uv.x), 0.0, 1.0);\n"
            "    float b = clamp(u_base + u_scale * (u_axis.z * (1.0 - uv.x) + (1.0 - u_axis.z) * (1.0 - uv.y)), 0.0, 1.0);\n"
            "    gl_FragColor = vec4(r, g, b, 1.0);\n"
            "}\n";

        program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource);
        program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource);
        program.link();

        return program;
    }

    void DrawFullscreenQuad()
    {
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(-1.0f, 1.0f);
        glVertex2f(1.0f, 1.0f);
        glEnd();
    }

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

void RenderBackground::render(const ndv::RenderContext& context)
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

    const int width = context.viewportWidth;
    const int height = context.viewportHeight;
    if (width <= 0 || height <= 0)
    {
        return;
    }

    const float axisX = std::fabs(context.view.m[2]);
    const float axisY = std::fabs(context.view.m[6]);
    const float axisZ = std::fabs(context.view.m[10]);

    const float base = 0.1f;
    const float scale = 0.8f;

    QOpenGLShaderProgram& program = BackgroundProgram();
    if (!program.isLinked())
    {
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    program.bind();
    program.setUniformValue("u_axis", QVector3D(axisX, axisY, axisZ));
    program.setUniformValue("u_resolution", QVector2D(static_cast<float>(width), static_cast<float>(height)));
    program.setUniformValue("u_base", base);
    program.setUniformValue("u_scale", scale);
    DrawFullscreenQuad();
    program.release();
    std::ostringstream fpsText;
    fpsText << "FPS: " << std::fixed << std::setprecision(1) << fps;
    DrawText2D(-0.95f, 0.9f, fpsText.str());
    glEnable(GL_DEPTH_TEST);
}
