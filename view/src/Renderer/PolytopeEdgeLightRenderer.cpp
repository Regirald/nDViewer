#include "PolytopeEdgeLightRenderer.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QVector4D>

PolytopeEdgeLightRenderer::~PolytopeEdgeLightRenderer()
{
    // The OpenGL context must still be current when GL resources are destroyed.
    // If this is not guaranteed, release resources explicitly before context teardown.
    if (m_program)
    {
        delete m_program;
        m_program = nullptr;
    }

    m_gl = nullptr;
    m_initialized = false;
}

void PolytopeEdgeLightRenderer::initializeGLResources()
{
    if (m_initialized)
        return;

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
        return;

    m_gl = ctx->extraFunctions();
    if (!m_gl)
        return;

    m_program = new QOpenGLShaderProgram();

    const char* vertexShader = R"(
        #version 330 core

        layout(location = 0) in vec3 position;

        uniform mat4 uView;
        uniform mat4 uProjection;

        void main()
        {
            // Keep vertices in homogeneous clip space.
            // The GPU performs clipping before perspective division.
            gl_Position = uProjection * uView * vec4(position, 1.0);
        }
    )";

    const char* fragmentShader = R"(
        #version 330 core

        out vec4 fragColor;

        void main()
        {
            fragColor = vec4(1.0, 1.0, 0.0, 1.0);
        }
    )";

    if (!m_program->addShaderFromSourceCode(
        QOpenGLShader::Vertex, vertexShader) ||
        !m_program->addShaderFromSourceCode(
            QOpenGLShader::Fragment, fragmentShader) ||
        !m_program->link())
    {
        qWarning() << "PolytopeEdgeLightRenderer shader error:"
            << m_program->log();

        delete m_program;
        m_program = nullptr;
        m_gl = nullptr;
        return;
    }

    // Create the vertex buffer and vertex array.
    m_gl->glGenVertexArrays(1, &m_vao);
    m_gl->glGenBuffers(1, &m_vbo);

    m_gl->glBindVertexArray(m_vao);
    m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Allocate space for the two 3D endpoints.
    m_gl->glBufferData(
        GL_ARRAY_BUFFER,
        2 * 3 * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW);

    m_gl->glEnableVertexAttribArray(0);
    m_gl->glVertexAttribPointer(
        0,                  // Attribute location
        3,                  // XYZ
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        nullptr);

    m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_gl->glBindVertexArray(0);

    m_initialized = true;
}

void PolytopeEdgeLightRenderer::render(
    const ndv::RenderContext& context)
{
    if (!m_initialized)
        initializeGLResources();

    if (!m_initialized || !m_program)
        return;

    // Original world-space endpoints. Do not convert to NDC.
    const float vertices[] = {
        static_cast<float>(m_a[0]),
        static_cast<float>(m_a[1]),
        static_cast<float>(m_a[2]),

        static_cast<float>(m_b[0]),
        static_cast<float>(m_b[1]),
        static_cast<float>(m_b[2])
    };

    m_program->bind();

    m_program->setUniformValue("uView", context.view);
    m_program->setUniformValue("uProjection", context.projection);

    m_gl->glBindVertexArray(m_vao);
    m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    m_gl->glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(vertices),
        vertices);

    m_gl->glLineWidth(2.0f);
    m_gl->glDrawArrays(GL_LINES, 0, 2);

    m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_gl->glBindVertexArray(0);

    m_program->release();
}