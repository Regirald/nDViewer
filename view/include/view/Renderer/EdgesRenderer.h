#pragma once
#include "Renderer/Renderer.h"
#include "model/cylinders.h"
#include "Renderer/RenderImplicitFunction.h"
#include "model/GeometryFromTopology.h"
#include <memory>

class EdgesRenderer : public Renderer
{
public:
	EdgesRenderer(PolytopeGeometry<3>* poly) : m_poly(poly) {}
	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry<3>* m_poly = nullptr;
	std::unique_ptr<CylindersCollection> m_cylinders;
	std::unique_ptr<RenderImplicitFunction<3>> m_renderer;
};
