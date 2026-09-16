#include "FacesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

void FacesRenderer::initializeGLResources()
{
	if (!m_poly)
		return;

	std::vector<std::pair<PlanesCollection::Point, PlanesCollection::Point>> planes;
	planes.reserve(m_poly->faces.size());

	// For each face compute a simple plane (point + normal) from the first three vertices
	for (const auto& f : m_poly->faces)
	{
		if (f.size() < 3) continue;
		auto a = m_poly->vertices[f[0]];
		auto b = m_poly->vertices[f[1]];
		auto c = m_poly->vertices[f[2]];
		// compute normal
		Vec<3, float> ab = { b[0]-a[0], b[1]-a[1], b[2]-a[2] };
		Vec<3, float> ac = { c[0]-a[0], c[1]-a[1], c[2]-a[2] };
		Vec<3, float> n = {
			ab[1]*ac[2] - ab[2]*ac[1],
			ab[2]*ac[0] - ab[0]*ac[2],
			ab[0]*ac[1] - ab[1]*ac[0]
		};
		// normalize
		float len = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		if (len > 1e-6f) { n[0]/=len; n[1]/=len; n[2]/=len; }
		planes.push_back({a, n});
	}

	m_planes = std::make_unique<PlanesCollection>(planes);
	m_renderer = std::make_unique<RenderImplicitFunction<3>>(m_planes.get());
	m_renderer->initializeGLResources();
}

void FacesRenderer::render(const ndv::RenderContext& context)
{
	if (!m_renderer)
		return;
	m_renderer->render(context);
}
