#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopeNode.h"
#include "model/PolytopeNodeLight.h"
#include "Renderer/PolytopeNodeLightRenderer.h"
#include "Renderer/RenderImplicitFunction.h"
#include "model/GeometryFromTopology.h"
#include <memory>
#include <vector>

class VerticesRenderer : public Renderer
{
public:
	VerticesRenderer(PolytopeGeometry3D* poly) : m_poly(poly) {}
	void initializeGLResources(bool fullShadedRendering = true);
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry3D* m_poly = nullptr;
	// Each renderer will own its PolytopeNode instance internally.
	std::vector<std::unique_ptr<Renderer>> m_renderers;
};
