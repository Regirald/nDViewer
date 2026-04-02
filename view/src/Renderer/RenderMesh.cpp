#include "RenderMesh.h"

#include <Windows.h>
#include <gl/GL.h>

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QVector3D>

namespace
{
    QOpenGLShaderProgram& MeshProgram()
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
            "uniform mat4 u_view;\n"
            "uniform mat4 u_projection;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = u_projection * u_view * gl_Vertex;\n"
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

        return program;
    }
}


template<size_t Dim>
void RenderMesh<Dim>::render(const ndv::RenderContext& context)
{
    if (!m_mesh)
    {
        return;
    }

    QOpenGLShaderProgram& program = MeshProgram();
    if (!program.isLinked())
    {
        return;
    }

    program.bind();
    program.setUniformValue("u_view", QMatrix4x4(context.view.m));
    program.setUniformValue("u_projection", QMatrix4x4(context.projection.m));
    program.setUniformValue("u_color", QVector3D(0.9f, 0.9f, 0.9f));

    // Draw m_mesh
    glBegin(GL_LINES);
    for (size_t i = 0; i < m_mesh->indices.size(); i += 3)
    {
        const auto& v0 = m_mesh->vertices[m_mesh->indices[i]];
        const auto& v1 = m_mesh->vertices[m_mesh->indices[i + 1]];
        const auto& v2 = m_mesh->vertices[m_mesh->indices[i + 2]];

        glVertex3f(v0[0], v0[1], 0.f);
        glVertex3f(v1[0], v1[1], 0.f);
        glVertex3f(v0[0], v0[1], 0.f);
        glVertex3f(v2[0], v2[1], 0.f);
        glVertex3f(v1[0], v1[1], 0.f);
        glVertex3f(v2[0], v2[1], 0.f);
    }
    glEnd();

    program.release();
}

template class RenderMesh<2>;
template class RenderMesh<3>;
