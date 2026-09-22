#include "VerticesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

#include "model/Sphere.h"


void VerticesRenderer::initializeGLResources()
{
	if (!m_poly)
		return;

	std::vector<PolytopeNodes::Point> centers;
	std::vector<float> radii;
	centers.reserve(m_poly->vertices.size());
	radii.reserve(m_poly->vertices.size());

	for (const auto& v : m_poly->vertices)
	{
		centers.push_back(v);
		radii.push_back(0.25f);
	}

	m_nodes = std::make_unique<PolytopeNodes>(centers, radii);

	m_renderer = std::make_unique<RenderImplicitFunction<3>>(m_nodes.release());
	m_renderer->initializeGLResources();
}

void VerticesRenderer::render(const ndv::RenderContext& context)
{
	if (!m_renderer)
		return;
	m_renderer->render(context);
}
