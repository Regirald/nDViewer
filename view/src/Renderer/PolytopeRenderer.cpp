#include "PolytopeRenderer.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <memory>
#include <unordered_map>


void PolytopeRendererProxy::defineDimension() {
	if (auto polytope2D = dynamic_cast<PolytopeGeometry<2>*>(m_polytopeAbstract)) {
		m_dim = Dim2;
		return;
	}
	else if (auto polytope3D = dynamic_cast<PolytopeGeometry<3>*>(m_polytopeAbstract)) {
		m_dim = Dim3;
		return;
	}
	else if (auto polytope4D = dynamic_cast<PolytopeGeometry<4>*>(m_polytopeAbstract)) {
		m_dim = Dim4;
		return;
	}
	else
	{
		m_dim = Unknown;
	}

}


void PolytopeRendererProxy::render(const ndv::RenderContext& context)
{
	if (!m_polytope3D)
	{
		m_schlegelProjection = new SchlegelProjection<4>(
			Vec<4, float>{ 0.0f, 0.0f, 0.0f, 1.0f }, // facet normal
			1.0f, // facet offset
			Vec<4, float>{ 0.0f, 0.0f, 0.0f, 2.0f } // projection center
		);
		switch (m_dim)
		{
		case PolytopeRendererProxy::Dim2:
		{
			auto polytope2D = dynamic_cast<PolytopeGeometry<2>*>(m_polytopeAbstract);
			m_polytope3D = new PolytopeGeometry<3>(polytope2D);

		}
		case PolytopeRendererProxy::Dim3:
		{
			auto polytope3D = dynamic_cast<PolytopeGeometry<3>*>(m_polytopeAbstract);
			m_polytope3D = new PolytopeGeometry<3>(polytope3D);

		}
		case PolytopeRendererProxy::Dim4:
		{
			auto polytope4D = dynamic_cast<PolytopeGeometry<4>*>(m_polytopeAbstract);
			m_polytope3D = new PolytopeGeometry<3>(polytope4D, m_schlegelProjection);


		}
		default:
			break;
		}
	}
	if (!m_polytopeRenderer)
		m_polytopeRenderer = std::make_unique<PolytopeRenderer>(m_polytope3D);
	m_polytopeRenderer->render(context);
}

PolytopeRenderer::PolytopeRenderer(PolytopeGeometry<3>* polytope)
	:  m_polytope(polytope)
{
	m_initialized = false;
}


void PolytopeRenderer::initializeGLResources(bool fullShadedRendering)
{
	if (!m_polytope)
		return;

	m_verticesRenderer = std::make_unique<VerticesRenderer>(m_polytope);
	m_edgesRenderer = std::make_unique<EdgesRenderer>(m_polytope);
	m_facesRenderer = std::make_unique<FacesRenderer>(m_polytope);

	m_verticesRenderer->initializeGLResources(fullShadedRendering);
	m_edgesRenderer->initializeGLResources(fullShadedRendering);
	if (fullShadedRendering)
	{
		m_facesRenderer->initializeGLResources();
	}

	m_accelerationStructure = std::make_unique<AccelerationStructureBVH>(m_polytope, 0.25f);
	m_accelerationStructure->build();

	m_initialized = true;
}

void PolytopeRenderer::render(const ndv::RenderContext& context)
{
	if (!m_initialized)
		initializeGLResources(context.fullShadedRendering);

	if (!m_initialized)
		return;

	m_verticesRenderer->render(context);
	//m_edgesRenderer->render(context);
	if (context.fullShadedRendering)
	{
		//m_facesRenderer->render(context);
	}
	
}

