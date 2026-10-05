#include "VerticesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

#include "model/Sphere.h"

void VerticesRenderer::initializeGLResources()
{
	if (!m_poly)
		return;

	// Clear any previous renderers
	m_renderers.clear();

	// Create one renderer per vertex; each renderer owns its PolytopeNode
	for (const auto& v : m_poly->vertices)
	{
		PolytopeNode::Point center = v;
		float r = 0.25f;

		auto node = std::make_unique<PolytopeNode>(center, r);
		auto renderer = std::make_unique<RenderImplicitFunction<3>>(node.release());
		renderer->initializeGLResources();

		m_renderers.push_back(std::move(renderer));
	}
}

void VerticesRenderer::render(const ndv::RenderContext& context)
{
	if (m_renderers.empty())
		return;

	for (auto& r : m_renderers)
	{
		if (r)
			r->render(context);
	}
}
