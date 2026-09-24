#include "RenderBackground.h"


#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QVector2D>
#include <QVector3D>

#include <QVector2D>

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
            R"(
            #version 330 core

            layout(location = 0) 
            in vec2 a_position;
            out vec2 v_uv;

            void main()
            {
                v_uv = a_position * 0.5 + 0.5;
                gl_Position = vec4(a_position, 0.0, 1.0);
            }
            )";

        const char* fragmentSource =
            R"( #version 120
            in vec2 v_uv;

            uniform mat4 u_invView;
            uniform mat4 u_invProjection;

            uniform vec3 u_Xaxis;
            uniform vec3 u_Yaxis;
            uniform vec3 u_Zaxis;

            uniform vec3 u_lightDir;

            uniform float u_base;
            uniform float u_scale;

            out vec4 FragColor;

            void main()
            {
                // NDC
                vec2 ndc = v_uv * 2.0 - 1.0;

                // NDC -> view space
                vec4 rayView4 = u_invProjection * vec4(ndc, -1.0, 1.0);

                rayView4 /= rayView4.w;

                vec3 rayView = normalize(rayView4.xyz);

                // View -> world
                vec3 rayWorld = normalize((u_invView * vec4(rayView, 0.0)).xyz);

                // World axes
                float dx = abs(dot(rayWorld, normalize(u_Xaxis)));
                float dy = abs(dot(rayWorld, normalize(u_Yaxis)));
                float dz = abs(dot(rayWorld, normalize(u_Zaxis)));
                
                vec3 baseColor =
                    clamp(
                        u_base +
                        u_scale * vec3(dx, dy, dz),
                        0.0,
                        1.0
                    );
                vec3 white = vec3(1.0, 1.0, 1.0);

                vec3 L = normalize(u_lightDir);

                float cosAngle = max(dot(rayWorld, L), 0.0);

                // Broad soft halo
                float halo = pow(cosAngle, 8.0);

                // Smaller bright core
                float core = pow(cosAngle, 64.0);

                // Combine
                vec3 color =
                    baseColor
                    + white * 0.15 * halo
                    + white * 0.80 * core;

                color = clamp(color, 0.0, 1.0);

                FragColor = vec4(color, 1.0);
            };
            )";

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

    void DrawAxisArrows3D(const QMatrix4x4& view, const QMatrix4x4& projection)
    {
        // Small 3D axis at world origin, rendered with current view/projection
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixf(projection.constData());

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(view.constData());

        const float length = 0.5f;
        const float headSize = 0.1f;

        glDisable(GL_LIGHTING);
        glLineWidth(2.0f);

        // X axis (red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(length, 0.0f, 0.0f);
        glEnd();

        glBegin(GL_TRIANGLES);
        glVertex3f(length, 0.0f, 0.0f);
        glVertex3f(length - headSize, 0.03f, 0.0f);
        glVertex3f(length - headSize, -0.03f, 0.0f);
        glEnd();

        // Y axis (green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, length, 0.0f);
        glEnd();

        glBegin(GL_TRIANGLES);
        glVertex3f(0.0f, length, 0.0f);
        glVertex3f(0.03f, length - headSize, 0.0f);
        glVertex3f(-0.03f, length - headSize, 0.0f);
        glEnd();

        // Z axis (blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, length);
        glEnd();

        glBegin(GL_TRIANGLES);
        glVertex3f(0.0f, 0.0f, length);
        glVertex3f(0.0f, 0.03f, length - headSize);
        glVertex3f(0.0f, -0.03f, length - headSize);
        glEnd();

        glLineWidth(1.0f);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
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

    QOpenGLShaderProgram& program = BackgroundProgram();
    if (!program.isLinked())
    {
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    program.bind();


    program.setUniformValue("u_invView", context.view.inverted());
    program.setUniformValue("u_invProjection", context.projection.inverted());
    program.setUniformValue("u_Xaxis", QVector3D(1.f, 0.0f, 0.0f));
    program.setUniformValue("u_Yaxis", QVector3D(0.0f, 1.f, 0.0f));
    program.setUniformValue("u_Zaxis", QVector3D(0.0f, 0.0f, 1.f));
    program.setUniformValue("u_lightDir", context.lightningDirection);
    program.setUniformValue("u_base", 0.1f);
    program.setUniformValue("u_scale", 0.8f);

    DrawFullscreenQuad();
    program.release();

    // Draw 3D axis gizmo at world origin using same camera
    glEnable(GL_DEPTH_TEST);
    DrawAxisArrows3D(context.view, context.projection);


    std::ostringstream fpsText;
    fpsText << "FPS: " << std::fixed << std::setprecision(1) << fps;
    DrawText2D(-0.95f, 0.9f, fpsText.str());
}
