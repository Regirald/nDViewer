#include "EdgesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

template <size_t Dim>
void EdgesRenderer<Dim>::initializeGLResources(bool fullShadedRendering)
{
	m_renderer.reset();

	if (!m_poly || m_poly->vertices.empty())
		return;

	std::vector<Vec<3, float>> vertices_a;
	std::vector<Vec<3, float>> vertices_b;


	for (const auto& e : m_poly->edges)
	{
		if (e.size() < 2) continue;
		vertices_a.push_back(m_poly->vertices[e[0]]);
		vertices_b.push_back(m_poly->vertices[e[1]]);
	}

	constexpr float radius = 0.1f;
	auto edgeSet = std::make_unique<PolytopeEdge>(vertices_a, vertices_b, radius);
	m_renderer = std::make_unique<RenderImplicitFunctionSet<3>>(std::move(edgeSet));
	m_renderer->initializeGLResources();
}

template <size_t Dim>
void EdgesRenderer<Dim>::render(const ndv::RenderContext& context)
{
	if (m_renderer)
		m_renderer->render(context);
}

template class EdgesRenderer<2>;
template class EdgesRenderer<3>;
template class EdgesRenderer<4>;
