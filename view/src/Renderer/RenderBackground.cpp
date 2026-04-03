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

#include "core/Math/Mat4.h"
#include "core/Math/Vec3.h"
#include "core/Math/Vec4.h"

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
            "uniform mat4 u_view;\n"
            "uniform mat4 u_projection;\n"
            "uniform mat4 u_invVP;\n"
            "uniform vec2 u_resolution;\n"
            "uniform vec3 u_axis;   // encodes how much camera aligns with world axes (optional)\n"
            "uniform float u_base;\n"
            "uniform float u_scale;\n"
            "void main()\n"
            "{\n"
            "    // Normalized device coordinates for this pixel\n"
            "    vec2 ndc = (gl_FragCoord.xy / u_resolution) * 2.0 - 1.0;\n"
            "    // Build clip-space position at some depth (e.g. z = -1 in clip)\n"
            "    vec4 clipPos = vec4(ndc, -1.0, 1.0);\n"
            "    // Inverse projection * view to get a world-space point\n"
            "    vec4 worldPos = u_invVP * clipPos;\n"
            "    worldPos /= worldPos.w;\n"
            "    // Direction from camera (origin in view space) to worldPos in world space\n"
            "    // Camera position is inverse of view applied to origin; however, to get\n"
            "    // a camera-space direction, transform worldPos by view\n"
            "    vec4 viewPos = u_view * worldPos;\n"
            "    vec3 dir = normalize(viewPos.xyz);\n"
            "\n"
            "    // World axes in view space\n"
            "    vec3 ax = normalize( (u_view * vec4(1.0, 0.0, 0.0, 0.0)).xyz );\n"
            "    vec3 ay = normalize( (u_view * vec4(0.0, 1.0, 0.0, 0.0)).xyz );\n"
            "    vec3 az = normalize( (u_view * vec4(0.0, 0.0, 1.0, 0.0)).xyz );\n"
            "\n"
            "    float dx = abs(dot(dir, ax));\n"
            "    float dy = abs(dot(dir, ay));\n"
            "    float dz = abs(dot(dir, az));\n"
            "\n"
            "    vec3 color = clamp(u_base + u_scale * vec3(dx, dy, dz), 0.0, 1.0);\n"
            "    gl_FragColor = vec4(color, 1.0);\n"
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

    ndv::math::Vec2 Normalize2D(const ndv::math::Vec2& value)
    {
        const float len = std::sqrt(value.x * value.x + value.y * value.y);
        if (len <= 0.0001f)
        {
            return {};
        }
        return { value.x / len, value.y / len };
    }

    bool GetArrowGeometry(const ndv::math::Vec2& origin, const ndv::math::Vec2& direction, float length, float headSize,
        ndv::math::Vec2& end, ndv::math::Vec2& base, ndv::math::Vec2& perp)
    {
        const ndv::math::Vec2 dir = Normalize2D(direction);
        if (dir.x == 0.0f && dir.y == 0.0f)
        {
            return false;
        }

        end = origin + dir * length;
        perp = { -dir.y, dir.x };
        base = end - dir * headSize;
        return true;
    }

    void DrawAxisArrows2D(const ndv::math::Mat4& view)
    {
        const ndv::math::Vec2 origin{ -0.85f, -0.85f };
        const float length = 0.2f;
        const float headSize = 0.04f;

        const ndv::math::Vec4 axisX = view * ndv::math::Vec4(1.0f, 0.0f, 0.0f, 0.0);
        const ndv::math::Vec4 axisY = view * ndv::math::Vec4(0.0f, 1.0f, 0.0f, 0.0);
        const ndv::math::Vec4 axisZ = view * ndv::math::Vec4(0.0f, 0.0f, 1.0f, 0.0);

        ndv::math::Vec2 end;
        ndv::math::Vec2 base;
        ndv::math::Vec2 perp;

        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glColor3f(1.0f, 0.0f, 0.0f);
        if (GetArrowGeometry(origin, { axisX.x, axisX.y }, length, headSize, end, base, perp))
        {
            glVertex2f(origin.x, origin.y);
            glVertex2f(end.x, end.y);
        }

        glColor3f(0.0f, 1.0f, 0.0f);
        if (GetArrowGeometry(origin, { axisY.x, axisY.y }, length, headSize, end, base, perp))
        {
            glVertex2f(origin.x, origin.y);
            glVertex2f(end.x, end.y);
        }

        glColor3f(0.0f, 0.0f, 1.0f);
        if (GetArrowGeometry(origin, { axisZ.x, axisZ.y }, length, headSize, end, base, perp))
        {
            glVertex2f(origin.x, origin.y);
            glVertex2f(end.x, end.y);
        }
        glEnd();

        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);
        if (GetArrowGeometry(origin, { axisX.x, axisX.y }, length, headSize, end, base, perp))
        {
            glVertex2f(end.x, end.y);
            glVertex2f(base.x + perp.x * headSize * 0.5f, base.y + perp.y * headSize * 0.5f);
            glVertex2f(base.x - perp.x * headSize * 0.5f, base.y - perp.y * headSize * 0.5f);
        }

        glColor3f(0.0f, 1.0f, 0.0f);
        if (GetArrowGeometry(origin, { axisY.x, axisY.y }, length, headSize, end, base, perp))
        {
            glVertex2f(end.x, end.y);
            glVertex2f(base.x + perp.x * headSize * 0.5f, base.y + perp.y * headSize * 0.5f);
            glVertex2f(base.x - perp.x * headSize * 0.5f, base.y - perp.y * headSize * 0.5f);
        }

        glColor3f(0.0f, 0.0f, 1.0f);
        if (GetArrowGeometry(origin, { axisZ.x, axisZ.y }, length, headSize, end, base, perp))
        {
            glVertex2f(end.x, end.y);
            glVertex2f(base.x + perp.x * headSize * 0.5f, base.y + perp.y * headSize * 0.5f);
            glVertex2f(base.x - perp.x * headSize * 0.5f, base.y - perp.y * headSize * 0.5f);
        }
        glEnd();

        glLineWidth(1.0f);
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


    // Pass view/projection like RenderMesh does, so background uses same camera
    const ndv::math::Mat4& view = context.view;
    const ndv::math::Mat4& projection = context.projection;
    QMatrix4x4 viewMatrix(
        view(0, 0), view(0, 1), view(0, 2), view(0, 3),
        view(1, 0), view(1, 1), view(1, 2), view(1, 3),
        view(2, 0), view(2, 1), view(2, 2), view(2, 3),
        view(3, 0), view(3, 1), view(3, 2), view(3, 3));
    QMatrix4x4 projectionMatrix(
        projection(0, 0), projection(0, 1), projection(0, 2), projection(0, 3),
        projection(1, 0), projection(1, 1), projection(1, 2), projection(1, 3),
        projection(2, 0), projection(2, 1), projection(2, 2), projection(2, 3),
        projection(3, 0), projection(3, 1), projection(3, 2), projection(3, 3));

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

    DrawAxisArrows2D(context.view);

    std::ostringstream fpsText;
    fpsText << "FPS: " << std::fixed << std::setprecision(1) << fps;
    DrawText2D(-0.95f, 0.9f, fpsText.str());
    glEnable(GL_DEPTH_TEST);
}
