#pragma once


#include "Renderer/Renderer.h"
#include "Renderer/VerticesRenderer.h"
#include "Renderer/EdgesRenderer.h"
#include "Renderer/FacesRenderer.h"
#include "model/GeometryFromTopology.h"
#include "AccelerationStructureBVH.h"


class AbstractPolytopeRenderer : public Renderer
{
public :

	virtual void render(const ndv::RenderContext& context) = 0;
	bool m_initialized = false;

};

class PolytopeRendererProxy : public Renderer
{

enum InternalDimension { Dim2, Dim3, Dim4, Unknown };
public:
	PolytopeRendererProxy(AbstractPolytopeGeometry* polytope = nullptr) : m_polytope(polytope) { defineDimension(); }
	void defineDimension();

	void render(const ndv::RenderContext& context);

	InternalDimension m_dim = Unknown;
	AbstractPolytopeGeometry* m_polytope = nullptr;
	std::unique_ptr<AbstractPolytopeRenderer> m_polytopeRenderer = nullptr;


};



template <size_t Dim>
class PolytopeRenderer : public AbstractPolytopeRenderer
{
public:
	PolytopeRenderer(PolytopeGeometry<Dim>* polytope);

	void initializeGLResources(bool fullShadedRendering = true);
	void render(const ndv::RenderContext& context) override;




	PolytopeGeometry<Dim>* m_polytope;
	std::unique_ptr<VerticesRenderer<Dim>> m_verticesRenderer;
	std::unique_ptr<EdgesRenderer<Dim>> m_edgesRenderer;
	std::unique_ptr<FacesRenderer<Dim>> m_facesRenderer;

	std::unique_ptr<PolytopeAccelerationStructure<Dim>> m_accelerationStructure;
};
