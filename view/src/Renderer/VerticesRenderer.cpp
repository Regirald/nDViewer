#include "VerticesRenderer.h"
#include "model/AccelerationStructureBVH.h"
#include "model/VerticeSet.h"
#include <utility>
#include <vector>

void VerticesRenderer::initializeGLResources(bool fullShadedRendering)
{
	m_renderer.reset();
	if (!m_poly || m_poly->vertices.empty())
		return;

	constexpr float radius = 0.25f;

	AccelerationStructureBVH accelerationStructure(m_poly, radius);
	accelerationStructure.build();


	auto verticeSet = std::make_unique<VerticeSet>(m_poly->vertices, accelerationStructure.nodes(), accelerationStructure.primitives(), radius);
	m_renderer = std::make_unique<RenderImplicitFunction<3>>(std::move(verticeSet));
	m_renderer->initializeGLResources();
}

void VerticesRenderer::render(const ndv::RenderContext& context)
{
	if (m_renderer)
		m_renderer->render(context);
}

