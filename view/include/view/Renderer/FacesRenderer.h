#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopesFaces.h"
#include "Renderer/RenderImplicitFunction.h"
#include "model/GeometryFromTopology.h"
#include <memory>
#include <vector>

class FacesRenderer : public Renderer
{
public:
	FacesRenderer(PolytopeGeometry3D* poly) : m_poly(poly) {}
	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry3D* m_poly = nullptr;
	// Each renderer owns a single PolytopeFace instance.
	std::vector<std::unique_ptr<RenderImplicitFunction<3>>> m_renderers;
};
