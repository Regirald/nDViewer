#include "RenderMesh.h"

#include <Windows.h>
#include <gl/GL.h>


template<size_t Dim>
void RenderMesh<Dim>::render(const ndv::RenderContext& context)
{
    (void)context;
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
}

template class RenderMesh<2>;
template class RenderMesh<3>;
