#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopesEdges.h"
#include "Renderer/RenderImplicitFunction.h"
#include "model/GeometryFromTopology.h"
#include <memory>
#include <vector>

class EdgesRenderer : public Renderer
{
public:
	EdgesRenderer(PolytopeGeometry<3>* poly) : m_poly(poly) {}
	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry<3>* m_poly = nullptr;
	// Each renderer owns a single PolytopeEdge instance.
	std::vector<std::unique_ptr<RenderImplicitFunction<3>>> m_renderers;
};
