#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopeEdge.h"
#include "Renderer/RenderImplicitFunctionSet.h"
#include "Renderer/PolytopeEdgeLightRenderer.h"
#include "model/GeometryFromTopology.h"
#include <memory>
#include <vector>

template <size_t Dim>
class EdgesRenderer : public Renderer
{
public:
	EdgesRenderer(PolytopeGeometry<Dim>* poly) : m_poly(poly) {}
	void initializeGLResources(bool fullShadedRendering = true);
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry<Dim>* m_poly = nullptr;
	std::unique_ptr<RenderImplicitFunctionSet<3>> m_renderer;
};
