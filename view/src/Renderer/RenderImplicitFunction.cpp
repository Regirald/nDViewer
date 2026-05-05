#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <vector>
#include "renderer/RenderImplicitFunction.h"
#include "model/ImplicitFunction.h"

template<size_t Dim>
RenderImplicitFunction<Dim>::RenderImplicitFunction(ImplicitFunction<Dim>* implicitFunction)
    : m_implicitFunction(implicitFunction)
{
}

template<size_t Dim>
void RenderImplicitFunction<Dim>::initializeGLResources()
{
    if (!m_implicitFunction)
        return;

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    gl = ctx->extraFunctions();
    gl->initializeOpenGLFunctions();

    // -------------------------
    // Generate SDF texture
    // -------------------------
    generateSDF();

    // -------------------------
    // Compile shaders
    // -------------------------
    if (!m_program.isLinked())
    {
        const char* vs = R"(
            #version 330 core

            layout(location = 0) in vec2 a_position;
            out vec2 v_uv;

            void main()
            {
                v_uv = a_position * 0.5 + 0.5;
                gl_Position = vec4(a_position, 0.0, 1.0);
            }
           )";
        // Build fragment shader by inserting parameter declarations right
        // after the #version directive provided by the fragment template.
        std::string fs = m_implicitFunction->getFragmentShader();
        const std::string params = m_implicitFunction->getGLSL_parameters();
        const std::string versionStr = "#version 330 core";
        size_t vpos = fs.find(versionStr);
        if (vpos != std::string::npos)
        {
            size_t newline = fs.find('\n', vpos);
            size_t insertPos = (newline != std::string::npos) ? newline + 1 : vpos + versionStr.size();
            fs.insert(insertPos, params);
        }
        else
        {
            // Fallback: prefix version and params
            fs = versionStr + std::string("\n") + params + fs;
        }
        

        bool truc0 = m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
        bool truc1 = m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs.c_str());
        bool truc2 = m_program.link();
    }

    // -------------------------
    // Fullscreen quad
    // -------------------------
    if (!m_fullscreenVAO)
    {
        float quad[] = {
            -1.f, -1.f,
             1.f, -1.f,
            -1.f,  1.f,
             1.f,  1.f
        };

        gl->glGenVertexArrays(1, &m_fullscreenVAO);
        gl->glGenBuffers(1, &m_fullscreenVBO);

        gl->glBindVertexArray(m_fullscreenVAO);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenVBO);
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

        gl->glEnableVertexAttribArray(0);
        gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        gl->glBindVertexArray(0);
    }

    m_initialized = true;
}

template<size_t Dim>
void RenderImplicitFunction<Dim>::generateSDF()
{
    const int N = m_resolution;
    std::vector<float> data(N * N * N);

    QVector3D size = m_boundsMax - m_boundsMin;

    for (int z = 0; z < N; ++z)
        for (int y = 0; y < N; ++y)
            for (int x = 0; x < N; ++x)
            {
                QVector3D uvw(
                    float(x) / (N - 1),
                    float(y) / (N - 1),
                    float(z) / (N - 1)
                );

                QVector3D p = m_boundsMin + QVector3D(
                    uvw.x() * size.x(),
                    uvw.y() * size.y(),
                    uvw.z() * size.z()
                );

                float d = 0.f;

                if constexpr (Dim == 2)
                    d = m_implicitFunction->evaluateSDF({ p.x(), p.y() });
                else
                    d = m_implicitFunction->evaluateSDF({ p.x(), p.y(), p.z() });

                data[x + y * N + z * N * N] = d;
            }

    uploadSDF(data);
}

template<size_t Dim>
void RenderImplicitFunction<Dim>::uploadSDF(const std::vector<float>& data)
{
    if (!m_sdfTexture)
        gl->glGenTextures(1, &m_sdfTexture);

    gl->glBindTexture(GL_TEXTURE_3D, m_sdfTexture);

    gl->glTexImage3D(
        GL_TEXTURE_3D,
        0,
        GL_R32F,
        m_resolution,
        m_resolution,
        m_resolution,
        0,
        GL_RED,
        GL_FLOAT,
        data.data()
    );

    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

template<size_t Dim>
void RenderImplicitFunction<Dim>::render(const ndv::RenderContext& context)
{
    if (!m_initialized)
        initializeGLResources();

    if (!m_initialized)
        return;

    m_program.bind();

    // Texture
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_3D, m_sdfTexture);
    m_program.setUniformValue("u_sdf", 0);

    // Let implicit function upload its uniforms
    if (m_implicitFunction)
        m_implicitFunction->setGLUniforms(m_program);

    // Bounds
    m_program.setUniformValue("u_boundsMin", m_boundsMin);
    m_program.setUniformValue("u_boundsMax", m_boundsMax);

    // Camera
    m_program.setUniformValue("u_invView", context.view.inverted());
    m_program.setUniformValue("u_invProjection", context.projection.inverted());
    m_program.setUniformValue("u_cameraPos", context.cameraPosition);

    // Parameters
    m_program.setUniformValue("u_maxDistance", 100.0f);
    m_program.setUniformValue("u_maxSteps", 128);
    m_program.setUniformValue("u_epsilon", 0.001f);

    // Draw fullscreen
    gl->glDisable(GL_DEPTH_TEST);
    gl->glBindVertexArray(m_fullscreenVAO);
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl->glBindVertexArray(0);

    m_program.release();
}

template class RenderImplicitFunction<2>;
template class RenderImplicitFunction<3>;