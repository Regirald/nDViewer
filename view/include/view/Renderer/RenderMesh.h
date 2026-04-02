#pragma once

#include "Renderer/Renderer.h"
#include "model/Mesh.h"

template <size_t Dim>
class RenderMesh : public Renderer
{
public:
	RenderMesh(Mesh<Dim>* mesh) { m_mesh = mesh; }
 void render(const ndv::RenderContext& context) override;

	Mesh<Dim>* m_mesh;
};