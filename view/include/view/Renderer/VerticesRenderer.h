#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopeNode.h"
#include "model/PolytopeNodeLight.h"
#include "Renderer/PolytopeNodeLightRenderer.h"
#include "Renderer/RenderImplicitFunction.h"
#include "model/GeometryFromTopology.h"
#include <memory>
#include <vector>

template <size_t Dim>
class VerticesRenderer : public Renderer
{
public:
	VerticesRenderer(PolytopeGeometry<Dim>* poly) : m_poly(poly) {}
	void initializeGLResources(bool fullShadedRendering = true);
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry<Dim>* m_poly = nullptr;
	std::vector<std::unique_ptr<Renderer>> m_renderers;
};
