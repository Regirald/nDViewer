#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopesNodes.h"
#include "Renderer/RenderImplicitFunction.h"
#include "model/GeometryFromTopology.h"
#include <memory>
#include <vector>

class VerticesRenderer : public Renderer
{
public:
	VerticesRenderer(PolytopeGeometry3D* poly) : m_poly(poly) {}
	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry3D* m_poly = nullptr;
	// Each renderer will own its PolytopeNode instance internally.
	std::vector<std::unique_ptr<RenderImplicitFunction<3>>> m_renderers;
};
