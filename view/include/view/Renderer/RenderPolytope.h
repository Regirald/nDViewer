#pragma once


#include "Renderer/Renderer.h"
#include "Renderer/VerticesRenderer.h"
#include "Renderer/EdgesRenderer.h"
#include "Renderer/FacesRenderer.h"
#include "model/GeometryFromTopology.h"

class RenderPolytope : public Renderer
{
public:
	RenderPolytope(PolytopeGeometry3D* polytope);

	void initializeGLResources(bool fullShadedRendering = true);
	void render(const ndv::RenderContext& context) override;


	bool m_initialized = false;


	PolytopeGeometry3D* m_polytope;
	std::unique_ptr<VerticesRenderer> m_verticesRenderer;
	std::unique_ptr<EdgesRenderer> m_edgesRenderer;
	std::unique_ptr<FacesRenderer> m_facesRenderer;
};
