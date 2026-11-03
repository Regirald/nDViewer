#include "PolytopeRenderer.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <memory>
#include <unordered_map>


void PolytopeRendererProxy::defineDimension() {
	if (auto polytope2D = dynamic_cast<PolytopeGeometry<2>*>(m_polytope)) {
		m_dim = Dim2;
		return;
	}
	else if (auto polytope3D = dynamic_cast<PolytopeGeometry<3>*>(m_polytope)) {
		m_dim = Dim3;
		return;
	}
	else if (auto polytope4D = dynamic_cast<PolytopeGeometry<4>*>(m_polytope)) {
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
	switch (m_dim)
	{
	case PolytopeRendererProxy::Dim2:
	{
		auto polytope2D = dynamic_cast<PolytopeGeometry<2>*>(m_polytope);
		if (polytope2D)
		{
			if (!m_polytopeRenderer)
				m_polytopeRenderer = std::make_unique<PolytopeRenderer<2>>(polytope2D);
			m_polytopeRenderer->render(context);
		}
		break;
	}
	case PolytopeRendererProxy::Dim3:
	{
		auto polytope3D = dynamic_cast<PolytopeGeometry<3>*>(m_polytope);
		if (polytope3D)
		{
			if (!m_polytopeRenderer)
				m_polytopeRenderer = std::make_unique<PolytopeRenderer<3>>(polytope3D);
			m_polytopeRenderer->render(context);
		}
		break;
	}
	case PolytopeRendererProxy::Dim4:
	{
		auto polytope4D = dynamic_cast<PolytopeGeometry<4>*>(m_polytope);
		if (polytope4D)
		{
			if (!m_polytopeRenderer)
				m_polytopeRenderer = std::make_unique<PolytopeRenderer<4>>(polytope4D);
			m_polytopeRenderer->render(context);
		}
		break;
	}
	default:
		break;
	}

}

template <size_t Dim>
PolytopeRenderer<Dim>::PolytopeRenderer(PolytopeGeometry<Dim>* polytope)
	: m_polytope(polytope)
{
	m_initialized = false;
}


template <size_t Dim>
void PolytopeRenderer<Dim>::initializeGLResources(bool fullShadedRendering)
{
	if (!m_polytope)
		return;

	m_verticesRenderer = std::make_unique<VerticesRenderer<Dim>>(m_polytope);
	m_edgesRenderer = std::make_unique<EdgesRenderer<Dim>>(m_polytope);
	m_facesRenderer = std::make_unique<FacesRenderer<Dim>>(m_polytope);

	m_verticesRenderer->initializeGLResources(fullShadedRendering);
	m_edgesRenderer->initializeGLResources(fullShadedRendering);
	if (fullShadedRendering)
	{
		m_facesRenderer->initializeGLResources();
	}

	m_accelerationStructure = std::make_unique<PolytopeAccelerationStructure<Dim>>(m_polytope);
	m_accelerationStructure->build();

	std::cout << "BVH nodes: "
		<< m_accelerationStructure->nodes().size()
		<< '\n';

	std::cout << "BVH primitives: "
		<< m_accelerationStructure->primitives().size()
		<< '\n';

	m_initialized = true;
}

template <size_t Dim>
void PolytopeRenderer<Dim>::render(const ndv::RenderContext& context)
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






template class PolytopeRenderer<2>;
template class PolytopeRenderer<3>;
template class PolytopeRenderer<4>;