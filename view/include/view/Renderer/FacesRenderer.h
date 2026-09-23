#pragma once
#include "Renderer/Renderer.h"
#include "model/PolytopesFaces.h"
#include "Renderer/RenderImplicitFunction.h"
#include "model/GeometryFromTopology.h"
#include <memory>

class FacesRenderer : public Renderer
{
public:
	FacesRenderer(PolytopeGeometry<3>* poly) : m_poly(poly) {}
	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry<3>* m_poly = nullptr;
	std::unique_ptr<PolytopesFaces> m_planes;
	std::unique_ptr<RenderImplicitFunction<3>> m_renderer;
};
