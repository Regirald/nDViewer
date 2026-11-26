#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <vector>
#include "renderer/RenderImplicitFunctionSet.h"
#include "model/ImplicitFunctionSet.h"

template<size_t Dim>
RenderImplicitFunctionSet<Dim>::RenderImplicitFunctionSet(std::unique_ptr<ImplicitFunctionSet<Dim>> implicitFunctionSet)
    : m_implicitFunctionSet(std::move(implicitFunctionSet))
{
}

template<size_t Dim>
void RenderImplicitFunctionSet<Dim>::initializeGLResources()
{
    gl = QOpenGLContext::currentContext()->extraFunctions();
    gl->initializeOpenGLFunctions();


    if (!m_program.isLinked())
    {
        std::string vs = m_implicitFunctionSet->getVertexShader();
        std::string fs = m_implicitFunctionSet->getFragmentShader();


        bool flag0 = m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs.c_str());
        bool flag1 = m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs.c_str());
        bool flag2 = m_program.link();

        assert(flag0 && flag1 && flag2);
    }

    // -------------------------
    // Per-primitive quad
    // -------------------------
    if (!m_fullscreenVAO)
    {
        const float quad[] = {
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
void RenderImplicitFunctionSet<Dim>::render(const ndv::RenderContext& context)
{
    if (!m_initialized)
        initializeGLResources();

    assert(m_program.bind());



    // Camera
    m_program.setUniformValue("u_cameraPos", context.cameraPosition);
    m_program.setUniformValue("u_lightDir", context.lightningDirection);
    m_program.setUniformValue("u_view", context.view);
    m_program.setUniformValue("u_invView", context.view.inverted());
    m_program.setUniformValue("u_projection", context.projection);
    m_program.setUniformValue("u_invProjection", context.projection.inverted());
    m_program.setUniformValue("u_viewProjection", context.projection * context.view);

    // Parameters
    m_program.setUniformValue("u_maxDistance", 100.0f);
    m_program.setUniformValue("u_maxSteps", 128);
    m_program.setUniformValue("u_epsilon", 0.001f);

	// Draw primitives
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);


    for (int i = 0; i < m_implicitFunctionSet->m_primitiveCount; ++i)
    {
		m_implicitFunctionSet->setGLUniforms(m_program, i);

        gl->glBindVertexArray(m_fullscreenVAO);

        gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    m_program.release();
}


template<size_t Dim>
RenderImplicitFunctionSet<Dim>::~RenderImplicitFunctionSet()
{
    // OpenGL resources must be deleted with a compatible context current.
    if (QOpenGLContext::currentContext() && gl)
    {
        if (m_fullscreenVBO)
            gl->glDeleteBuffers(1, &m_fullscreenVBO);

        if (m_fullscreenVAO)
            gl->glDeleteVertexArrays(1, &m_fullscreenVAO);
    }
}

template class RenderImplicitFunctionSet<2>;
template class RenderImplicitFunctionSet<3>;
template class RenderImplicitFunctionSet<4>;