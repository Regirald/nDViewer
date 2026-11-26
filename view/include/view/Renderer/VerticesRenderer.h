#pragma once
#include "Renderer/Renderer.h"
#include "Renderer/RenderImplicitFunctionSet.h"
#include "model/GeometryFromTopology.h"
#include <memory>

template <size_t Dim>
class VerticesRenderer : public Renderer
{
public:
	VerticesRenderer(PolytopeGeometry<Dim>* poly) : m_poly(poly) {}
	void initializeGLResources(bool fullShadedRendering = true);
	void render(const ndv::RenderContext& context) override;

private:
	PolytopeGeometry<Dim>* m_poly = nullptr;
	std::unique_ptr<RenderImplicitFunctionSet<3>> m_renderer;
};
