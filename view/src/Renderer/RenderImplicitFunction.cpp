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
void RenderImplicitFunction<Dim>::render(const ndv::RenderContext& context)
{
    if (!m_initialized)
        initializeGLResources();

    if (!m_initialized)
        return;

    m_program.bind();


    // Let implicit function upload its uniforms
    if (m_implicitFunction)
        m_implicitFunction->setGLUniforms(m_program);

    // Bounds
    m_program.setUniformValue("u_boundsMin", m_boundsMin);
    m_program.setUniformValue("u_boundsMax", m_boundsMax);

    // Camera
    m_program.setUniformValue("u_cameraPos", context.cameraPosition);
    m_program.setUniformValue("u_view", context.view);
    m_program.setUniformValue("u_invView", context.view.inverted());
    m_program.setUniformValue("u_projection", context.projection);
    m_program.setUniformValue("u_invProjection", context.projection.inverted());

    // Parameters
    m_program.setUniformValue("u_maxDistance", 100.0f);
    m_program.setUniformValue("u_maxSteps", 128);
    m_program.setUniformValue("u_epsilon", 0.001f);

    // Draw fullscreen
    gl->glEnable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glBindVertexArray(m_fullscreenVAO);
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl->glBindVertexArray(0);

    m_program.release();
}

template class RenderImplicitFunction<2>;
template class RenderImplicitFunction<3>;