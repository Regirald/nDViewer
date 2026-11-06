#include "VerticesRenderer.h"
#include "model/AccelerationStructureBVH.h"
#include "model/VerticeSet.h"
#include <utility>
#include <vector>

template <size_t Dim>
void VerticesRenderer<Dim>::initializeGLResources(bool fullShadedRendering)
{
	m_renderer.reset();
	if (!m_poly || m_poly->vertices.empty())
		return;

	constexpr float radius = 0.25f;

	AccelerationStructureBVH<Dim> accelerationStructure(m_poly, radius);
	accelerationStructure.build();


	auto verticeSet = std::make_unique<VerticeSet<Dim>>(m_poly->vertices, accelerationStructure.nodes(), accelerationStructure.primitives(), radius);
	m_renderer = std::make_unique<RenderImplicitFunction<Dim>>(std::move(verticeSet));
	m_renderer->initializeGLResources();
}

template <size_t Dim>
void VerticesRenderer<Dim>::render(const ndv::RenderContext& context)
{
	if (m_renderer)
		m_renderer->render(context);
}

template class VerticesRenderer<2>;
template class VerticesRenderer<3>;
template class VerticesRenderer<4>;
