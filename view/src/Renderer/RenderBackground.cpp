#include "RenderBackground.h"

#include <Windows.h>
#include <gl/GL.h>

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
            #version 120
            void main()
            {
                gl_Position = vec4(gl_Vertex.xy, 0.0, 1.0);
            };)";

        const char* fragmentSource =
            R"( #version 120
            uniform mat4 u_view;
            uniform mat4 u_projection;
            uniform mat4 u_invVP;
            uniform vec2 u_resolution;
            uniform vec3 u_axis;   // encodes how much camera aligns with world axes (optional)
            uniform float u_base;
            uniform float u_scale;
            void main()
            {
                // Normalized device coordinates for this pixel
                vec2 ndc = (gl_FragCoord.xy / u_resolution) * 2.0 - 1.0;
                // Build clip-space position at some depth (e.g. z = -1 in clip)
                vec4 clipPos = vec4(ndc, -1.0, 1.0);
                // Inverse projection * view to get a world-space point
                vec4 worldPos = u_invVP * clipPos;
                worldPos /= worldPos.w;
                // Direction from camera (origin in view space) to worldPos in world space
                // Camera position is inverse of view applied to origin; however, to get
                // a camera-space direction, transform worldPos by view
                vec4 viewPos = u_view * worldPos;
                vec3 dir = normalize(viewPos.xyz);
            
                // World axes in view space
                vec3 ax = normalize( (u_view * vec4(1.0, 0.0, 0.0, 0.0)).xyz );
                vec3 ay = normalize( (u_view * vec4(0.0, 1.0, 0.0, 0.0)).xyz );
                vec3 az = normalize( (u_view * vec4(0.0, 0.0, 1.0, 0.0)).xyz );
            
                float dx = abs(dot(dir, ax));
                float dy = abs(dot(dir, ay));
                float dz = abs(dot(dir, az));
            
                vec3 color = clamp(u_base + u_scale * vec3(dx, dy, dz), 0.0, 1.0);
                gl_FragColor = vec4(color, 1.0);
            };)";

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

    const float* viewData = context.view.constData();
    const float axisX = std::fabs(viewData[2]);
    const float axisY = std::fabs(viewData[6]);
    const float axisZ = std::fabs(viewData[10]);

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


    // Pass view/projection like RenderMesh does, so background uses same camera
    const QMatrix4x4& viewMatrix = context.view;
    const QMatrix4x4& projectionMatrix = context.projection;

    QMatrix4x4 invVP = (projectionMatrix * viewMatrix).inverted();


    program.setUniformValue("u_axis", QVector3D(axisX, axisY, axisZ));
    program.setUniformValue("u_resolution", QVector2D(static_cast<float>(width), static_cast<float>(height)));
    program.setUniformValue("u_view", viewMatrix);
    program.setUniformValue("u_projection", projectionMatrix);
    program.setUniformValue("u_invVP", invVP);
    program.setUniformValue("u_base", base);
    program.setUniformValue("u_scale", scale);

    DrawFullscreenQuad();
    program.release();

    // Draw 3D axis gizmo at world origin using same camera
    DrawAxisArrows3D(context.view, context.projection);

    std::ostringstream fpsText;
    fpsText << "FPS: " << std::fixed << std::setprecision(1) << fps;
    DrawText2D(-0.95f, 0.9f, fpsText.str());
    glEnable(GL_DEPTH_TEST);
}
