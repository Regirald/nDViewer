#include "EdgesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

void EdgesRenderer::initializeGLResources()
{
	if (!m_poly)
		return;

	std::vector<std::pair<PolytopeEdges::Point, PolytopeEdges::Point>> segments;
	std::vector<float> radii;

	segments.reserve(m_poly->edges.size());
	radii.reserve(m_poly->edges.size());

	for (const auto& e : m_poly->edges)
	{
		if (e.size() < 2) continue;
		auto a = m_poly->vertices[e[0]];
		auto b = m_poly->vertices[e[1]];
		segments.push_back({a, b});
		radii.push_back(0.1f);
	}

	m_edges = std::make_unique<PolytopeEdges>(segments, radii);
	m_renderer = std::make_unique<RenderImplicitFunction<3>>(m_edges.release());
	m_renderer->initializeGLResources();
}

void EdgesRenderer::render(const ndv::RenderContext& context)
{
	if (!m_renderer)
		return;
	m_renderer->render(context);
}
