#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopesNodes.h"
#include "Renderer/RenderImplicitFunction.h"
#include "model/GeometryFromTopology.h"
#include <memory>
#include <vector>

template <size_t Dim>
class VerticesRenderer : public Renderer
{
public:
	VerticesRenderer(PolytopeGeometry<Dim>* poly) : m_poly(poly) {}
	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry<Dim>* m_poly = nullptr;
	// Each renderer will own its PolytopeNode instance internally.
	std::vector<std::unique_ptr<RenderImplicitFunction<3>>> m_renderers;
};
