#include "EdgesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

template <size_t Dim>
void EdgesRenderer<Dim>::initializeGLResources()
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
		auto renderer = std::make_unique<RenderImplicitFunction<3>>(edge.release());
		renderer->initializeGLResources();

		m_renderers.push_back(std::move(renderer));
	}
}

template <size_t Dim>
void EdgesRenderer<Dim>::render(const ndv::RenderContext& context)
{
	if (m_renderers.empty())
		return;

	for (auto& r : m_renderers)
	{
		if (r)
			r->render(context);
	}
}

template class EdgesRenderer<2>;
template class EdgesRenderer<3>;