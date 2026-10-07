#include "EdgesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

void EdgesRenderer::initializeGLResources()
{
	if (!m_poly)
		return;

	m_renderers.clear();

	for (const auto& e : m_poly->edges)
	{
		if (e.size() < 2) continue;
		auto a = m_poly->vertices[e[0]];
		auto b = m_poly->vertices[e[1]];

		std::pair<PolytopeEdge::Point, PolytopeEdge::Point> seg{ a, b };
		float r = 0.1f;

		auto edge = std::make_unique<PolytopeEdge>(seg, r);
		auto renderer = std::make_unique<RenderImplicitFunction<3>>(std::move(edge));
		renderer->initializeGLResources();

		m_renderers.push_back(std::move(renderer));
	}
}

void EdgesRenderer::render(const ndv::RenderContext& context)
{
	if (m_renderers.empty())
		return;

	for (auto& r : m_renderers)
	{
		if (r)
			r->render(context);
	}
}
