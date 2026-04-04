#include "RenderTargetSphere.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVector3D>

namespace
{
    QOpenGLShaderProgram& TargetSphereProgram()
    {
        static QOpenGLShaderProgram program;
        static bool initialized = false;

        if (!initialized)
        {
            initialized = true;

            const char* vertexSource =
                "#version 120\n"
                "uniform mat4 u_view;\n"
                "uniform mat4 u_projection;\n"
                "uniform vec3 u_center;\n"
                "attribute vec3 a_position;\n"
                "void main()\n"
                "{\n"
                "    vec3 worldPos = u_center + a_position;\n"
                "    gl_Position = u_projection * u_view * vec4(worldPos, 1.0);\n"
                "}\n";

            const char* fragmentSource =
                "#version 120\n"
                "uniform vec3 u_color;\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = vec4(u_color, 1.0);\n"
                "}\n";

            program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource);
            program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource);
            program.link();
        }

        return program;
    }
}

RenderTargetSphere::RenderTargetSphere() = default;

RenderTargetSphere::~RenderTargetSphere()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
        return;

    QOpenGLFunctions* f = ctx->functions();
    if (!f)
        return;

    if (m_vbo)
        f->glDeleteBuffers(1, &m_vbo);
    if (m_ebo)
        f->glDeleteBuffers(1, &m_ebo);
}

void RenderTargetSphere::initializeGLResources()
{
    if (!m_gl)
        return;

    // Simple low-res sphere: icosahedron-ish approximation could be used; here a unit sphere
    // represented by latitude/longitude subdivision.

    const int stacks = 8;
    const int slices = 16;
    const float radius = 0.05f;

    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    for (int i = 0; i <= stacks; ++i)
    {
        float v = static_cast<float>(i) / static_cast<float>(stacks);
        float phi = v * 3.1415926535f;
        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        for (int j = 0; j <= slices; ++j)
        {
            float u = static_cast<float>(j) / static_cast<float>(slices);
            float theta = u * 2.0f * 3.1415926535f;
            float cosTheta = std::cos(theta);
            float sinTheta = std::sin(theta);

            float x = radius * sinPhi * cosTheta;
            float y = radius * cosPhi;
            float z = radius * sinPhi * sinTheta;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            indices.push_back(static_cast<uint32_t>(first));
            indices.push_back(static_cast<uint32_t>(second));
            indices.push_back(static_cast<uint32_t>(first + 1));

            indices.push_back(static_cast<uint32_t>(second));
            indices.push_back(static_cast<uint32_t>(second + 1));
            indices.push_back(static_cast<uint32_t>(first + 1));
        }
    }

    m_gl->glGenBuffers(1, &m_vbo);
    m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    m_gl->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    m_gl->glGenBuffers(1, &m_ebo);
    m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    m_indexCount = static_cast<int>(indices.size());
    m_initialized = true;
}

void RenderTargetSphere::render(const ndv::RenderContext& context)
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
        return;

    if (!m_gl)
        m_gl = ctx->functions();

    if (!m_initialized)
    {
        initializeGLResources();
        if (!m_initialized)
            return;
    }

    QOpenGLShaderProgram& program = TargetSphereProgram();
    if (!program.isLinked())
        return;

    program.bind();

    const QMatrix4x4& viewMatrix = context.view;
    const QMatrix4x4& projectionMatrix = context.projection;

    program.setUniformValue("u_view", viewMatrix);
    program.setUniformValue("u_projection", projectionMatrix);
    program.setUniformValue("u_color", QVector3D(1.0f, 0.2f, 0.2f));

    // Use camera target as center
    program.setUniformValue("u_center", context.targetPosition);

    int posLoc = program.attributeLocation("a_position");
    if (posLoc < 0)
    {
        program.release();
        return;
    }

    program.enableAttributeArray(posLoc);

    m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    m_gl->glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    m_gl->glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    program.disableAttributeArray(posLoc);
    program.release();
}
