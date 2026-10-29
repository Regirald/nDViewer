#include "RenderPolytope.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <memory>
#include <unordered_map>

// ------------------------------------------------------------
// Constructor (NO OpenGL here)
// ------------------------------------------------------------
RenderPolytope::RenderPolytope(PolytopeGeometry3D* polytope)
	: m_polytope(polytope)
{
	m_initialized = false;
}

// ------------------------------------------------------------
// GPU initialization (called lazily when context is valid)
// ------------------------------------------------------------
void RenderPolytope::initializeGLResources(bool fullShadedRendering)
{
	if (!m_polytope)
		return;

	m_verticesRenderer = std::make_unique<VerticesRenderer>(m_polytope);
	m_edgesRenderer = std::make_unique<EdgesRenderer>(m_polytope);
	m_facesRenderer = std::make_unique<FacesRenderer>(m_polytope);

	m_verticesRenderer->initializeGLResources(fullShadedRendering);
	m_edgesRenderer->initializeGLResources(fullShadedRendering);
	m_facesRenderer->initializeGLResources();

	m_initialized = true;
}

// ------------------------------------------------------------
// Render
// ------------------------------------------------------------
void RenderPolytope::render(const ndv::RenderContext& context)
{
	if (!m_initialized)
		initializeGLResources(context.fullShadedRendering);

	if (!m_initialized)
		return;

	m_verticesRenderer->render(context);
	m_edgesRenderer->render(context);
	if (context.fullShadedRendering)
	{
		m_facesRenderer->render(context);
	}
	
}



