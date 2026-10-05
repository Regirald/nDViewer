#include "FacesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

void FacesRenderer::initializeGLResources()
{
	if (!m_poly)
		return;

	m_renderers.clear();

	for (const auto& face : m_poly->faces)
	{
		std::vector<PolytopeFace::Point> verts;
		verts.reserve(face.size());
		for (int idx : face)
			verts.push_back(m_poly->vertices[idx]);

		auto plane = std::make_unique<PolytopeFace>(verts);
		auto renderer = std::make_unique<RenderImplicitFunction<3>>(plane.release());
		renderer->initializeGLResources();
		renderer->m_isTransparent = true;  // Mark face renderers as transparent

		m_renderers.push_back(std::move(renderer));
	}
}

void FacesRenderer::render(const ndv::RenderContext& context)
{
	if (m_renderers.empty())
		return;

	for (auto& r : m_renderers)
	{
		if (r)
			r->render(context);
	}
}
