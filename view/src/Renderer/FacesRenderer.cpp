#include "FacesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

void FacesRenderer::initializeGLResources()
{
	if (!m_poly)
		return;

	m_planes = std::make_unique<PolytopesFaces>(m_poly->faces, m_poly->vertices);

	m_renderer = std::make_unique<RenderImplicitFunction<3>>(m_planes.release());
	m_renderer->initializeGLResources();
}

void FacesRenderer::render(const ndv::RenderContext& context)
{
	if (!m_renderer)
		return;

	m_renderer->render(context);
}