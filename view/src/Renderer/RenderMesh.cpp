#include "RenderMesh.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QVector3D>
    

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
    if (!m_mesh)
        return;

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    gl = ctx->extraFunctions();
    gl->initializeOpenGLFunctions();

    // -------------------------
    // 1. Compile shader
    // -------------------------
    if (!m_program.isLinked())
    {
        const char* vs =
            "#version 330 core\n"
            "layout(location = 0) in vec3 a_position;\n"
            "uniform mat4 u_view;\n"
            "uniform mat4 u_projection;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = u_projection * u_view * vec4(a_position, 1.0);\n"
            "}\n";

        const char* fs =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "uniform vec3 u_color;\n"
            "void main()\n"
            "{\n"
            "    FragColor = vec4(u_color, 1.0);\n"
            "}\n";

        m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
        m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
        m_program.link();
    }

    // -------------------------
    // 2. Flatten vertex data
    // -------------------------
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

    std::vector<uint32_t> indices32(m_mesh->indices.begin(), m_mesh->indices.end());

    m_indexCount = static_cast<int>(indices32.size());

    // -------------------------
    // 3. Create VAO FIRST
    // -------------------------
    gl->glGenVertexArrays(1, &vao);
    gl->glBindVertexArray(vao);

    // -------------------------
    // 4. Create & fill VBO
    // -------------------------
    gl->glGenBuffers(1, &vbo);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    gl->glBufferData(GL_ARRAY_BUFFER,
        flatVertices.size() * sizeof(float),
        flatVertices.data(),
        GL_STATIC_DRAW);

    // -------------------------
    // 5. Create & fill EBO
    // -------------------------
    gl->glGenBuffers(1, &ebo);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices32.size() * sizeof(uint32_t),
        indices32.data(),
        GL_STATIC_DRAW);

    // -------------------------
    // 6. Define vertex layout
    // -------------------------
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(
        0,              // location = 0
        3,              // vec3
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    // -------------------------
    // 7. Unbind VAO (important)
    // -------------------------
    gl->glBindVertexArray(0);

    m_initialized = true;
}

// ------------------------------------------------------------
// Render
// ------------------------------------------------------------
template<size_t Dim>
void RenderMesh<Dim>::render(const ndv::RenderContext& context)
{
    if (!m_initialized)
        initializeGLResources();

    if (!m_initialized)
        return;

    gl->glEnable(GL_DEPTH_TEST);
    gl->glDepthFunc(GL_LESS);

    m_program.bind();

    m_program.setUniformValue("u_view", context.view);
    m_program.setUniformValue("u_projection", context.projection);
    m_program.setUniformValue("u_color", QVector3D(0.9f, 0.9f, 0.9f));

    gl->glBindVertexArray(vao);
    gl->glDrawElements(GL_LINES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    gl->glBindVertexArray(0);

    m_program.release();
}

// ------------------------------------------------------------
// Destructor (cleanup)
// ------------------------------------------------------------
template<size_t Dim>
RenderMesh<Dim>::~RenderMesh()
{
    if (!gl)
        return;

    if (vbo) gl->glDeleteBuffers(1, &vbo);
    if (ebo) gl->glDeleteBuffers(1, &ebo);
    if (vao) gl->glDeleteVertexArrays(1, &vao);
}

// ------------------------------------------------------------
// Explicit instantiation
// ------------------------------------------------------------
template class RenderMesh<2>;
template class RenderMesh<3>;