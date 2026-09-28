#include "VerticesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

#include "model/Sphere.h"

template <size_t Dim>
void VerticesRenderer<Dim>::initializeGLResources()
{
	if (!m_poly)
		return;

	// Clear any previous renderers
	m_renderers.clear();

	// Create one renderer per vertex; each renderer owns its PolytopeNode
	for (const Vector<Dim>& v : m_poly->vertices)
	{
		PolytopeNode::Point center = v;
		float r = 0.25f;

		auto node = std::make_unique<PolytopeNode>(center, r);
		auto renderer = std::make_unique<RenderImplicitFunction<3>>(node.release());
		renderer->initializeGLResources();

		m_renderers.push_back(std::move(renderer));
	}
}

template <size_t Dim>
void VerticesRenderer<Dim>::render(const ndv::RenderContext& context)
{
	if (m_renderers.empty())
		return;

	for (auto& r : m_renderers)
	{
		if (r)
			r->render(context);
	}
}


template class VerticesRenderer<2>;
template class VerticesRenderer<3>;