#include "RenderMesh.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVector3D>

// ------------------------------------------------------------
// Shader (kept simple, but NOT tied to GL init timing)
// ------------------------------------------------------------
namespace
{
    QOpenGLShaderProgram& MeshProgram()
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
                "attribute vec3 a_position;\n"
                "void main()\n"
                "{\n"
                "    gl_Position = u_projection * u_view * vec4(a_position, 1.0);\n"
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

// ------------------------------------------------------------
// Constructor (NO OpenGL here)
// ------------------------------------------------------------
template<size_t Dim>
RenderMesh<Dim>::RenderMesh(Mesh<Dim>* mesh)
    : m_mesh(mesh)
{
    vbo = 0;
    ebo = 0;
    gl = nullptr;
    m_initialized = false;
}

// ------------------------------------------------------------
// GPU initialization (called lazily when context is valid)
// ------------------------------------------------------------
template<size_t Dim>
void RenderMesh<Dim>::initializeGLResources()
{
    if (!gl || !m_mesh)
        return;

    // ---- Flatten vertices safely (supports Dim=2 or 3) ----
    std::vector<float> flatVertices;
    flatVertices.reserve(m_mesh->vertices.size() * 3);

    for (const auto& v : m_mesh->vertices)
    {
        flatVertices.push_back(v[0]);
        flatVertices.push_back(v[1]);

        if constexpr (Dim == 3)
            flatVertices.push_back(v[2]);
        else
            flatVertices.push_back(0.0f);
    }

    // ---- Ensure indices are uint32 ----
    std::vector<uint32_t> indices32;
    indices32.reserve(m_mesh->indices.size());

    for (auto idx : m_mesh->indices)
    {
        indices32.push_back(static_cast<uint32_t>(idx));
    }

    // ---- Create buffers ----
    gl->glGenBuffers(1, &vbo);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl->glBufferData(GL_ARRAY_BUFFER,
        flatVertices.size() * sizeof(float),
        flatVertices.data(),
        GL_STATIC_DRAW);

    gl->glGenBuffers(1, &ebo);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices32.size() * sizeof(uint32_t),
        indices32.data(),
        GL_STATIC_DRAW);

    m_indexCount = static_cast<int>(indices32.size());
    m_initialized = true;
}

// ------------------------------------------------------------
// Render
// ------------------------------------------------------------
template<size_t Dim>
void RenderMesh<Dim>::render(const ndv::RenderContext& context)
{
    if (!m_mesh)
        return;

    // ---- Ensure GL context ----
    if (!gl)
    {
        QOpenGLContext* ctx = QOpenGLContext::currentContext();
        if (!ctx)
            return;

        gl = ctx->functions();
    }

    // ---- Lazy init ----
    if (!m_initialized)
    {
        initializeGLResources();
        if (!m_initialized)
            return;
    }

    QOpenGLShaderProgram& program = MeshProgram();
    if (!program.isLinked())
        return;

    program.bind();

    // ---- Set uniforms ----
    const QMatrix4x4& viewMatrix = context.view;
    const QMatrix4x4& projectionMatrix = context.projection;
    program.setUniformValue("u_view", viewMatrix);
    program.setUniformValue("u_projection", projectionMatrix);
    program.setUniformValue("u_color", QVector3D(0.9f, 0.9f, 0.9f));

    // ---- Bind vertex buffer ----
    int posLoc = program.attributeLocation("a_position");
    if (posLoc < 0)
    {
        program.release();
        return;
    }

    program.enableAttributeArray(posLoc);

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl->glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // ---- Draw ----
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    gl->glDrawElements(GL_LINES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    program.disableAttributeArray(posLoc);
    program.release();
}

// ------------------------------------------------------------
// Destructor (cleanup)
// ------------------------------------------------------------
template<size_t Dim>
RenderMesh<Dim>::~RenderMesh()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx)
        return;

    QOpenGLFunctions* f = ctx->functions();
    if (!f)
        return;

    if (vbo)
        f->glDeleteBuffers(1, &vbo);
    if (ebo)
        f->glDeleteBuffers(1, &ebo);
}

// ------------------------------------------------------------
// Explicit instantiation
// ------------------------------------------------------------
template class RenderMesh<2>;
template class RenderMesh<3>;