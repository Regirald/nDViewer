#pragma once

#include "Render.h"
#include "model/Mesh.h"

template <size_t Dim>
class RenderMesh : public Render
{
public:
	RenderMesh(const Mesh<Dim>& mesh) { m_mesh = mesh; }
	void render() override;

	Mesh<Dim>* m_mesh;
};