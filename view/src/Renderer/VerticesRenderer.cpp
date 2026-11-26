#include "VerticesRenderer.h"
#include "model/VerticeSet.h"
#include <utility>
#include <vector>

template <size_t Dim>
void VerticesRenderer<Dim>::initializeGLResources(bool fullShadedRendering)
{
	m_renderer.reset();
	if (!m_poly || m_poly->vertices.empty())
		return;

	//Vertices Conversion to fit dimension 3 TODO: add projection system
	std::vector<Vec<3, float>> vertices;
	for (const auto& vertex : m_poly->vertices)
	{
		vertices.push_back(vertex);
	}


	constexpr float m_radius = 0.25f;
	auto verticeSet = std::make_unique<VerticeSet>(vertices, m_radius);
	m_renderer = std::make_unique<RenderImplicitFunctionSet<3>>(std::move(verticeSet));
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
