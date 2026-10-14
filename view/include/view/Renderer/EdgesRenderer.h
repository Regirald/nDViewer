#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopeEdge.h"
#include "Renderer/RenderImplicitFunction.h"
#include "Renderer/PolytopeEdgeLightRenderer.h"
#include "model/GeometryFromTopology.h"
#include <memory>
#include <vector>

class EdgesRenderer : public Renderer
{
public:
	EdgesRenderer(PolytopeGeometry3D* poly) : m_poly(poly) {}
	void initializeGLResources(bool fullShadedRendering = true);
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry3D* m_poly = nullptr;
	// Each renderer will own a renderer instance (either full SDF renderer or a lightweight renderer).
	std::vector<std::unique_ptr<Renderer>> m_renderers;
};
