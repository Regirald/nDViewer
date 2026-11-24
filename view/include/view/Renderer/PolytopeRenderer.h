#pragma once


#include "Renderer/Renderer.h"
#include "Renderer/VerticesRenderer.h"
#include "Renderer/EdgesRenderer.h"
#include "Renderer/FacesRenderer.h"
#include "model/ProjectionSystem.h"
#include "model/GeometryFromTopology.h"
#include "model/AccelerationStructureBVH.h"


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
	PolytopeRendererProxy(AbstractPolytopeGeometry* polytopeAbstract = nullptr) : m_polytopeAbstract(polytopeAbstract) { defineDimension(); }
	void defineDimension();

	void render(const ndv::RenderContext& context);

	InternalDimension m_dim = Unknown;

	AbstractPolytopeGeometry* m_polytopeAbstract = nullptr;
	PolytopeGeometry<3>* m_polytope3D = nullptr;
	SchlegelProjection<4>* m_schlegelProjection = nullptr;

	std::unique_ptr<AbstractPolytopeRenderer> m_polytopeRenderer = nullptr;


};



class PolytopeRenderer : public AbstractPolytopeRenderer
{
public:
	PolytopeRenderer(PolytopeGeometry<3>* polytope);

	void initializeGLResources(bool fullShadedRendering = true);
	void render(const ndv::RenderContext& context) override;



	PolytopeGeometry<3>* m_polytope;
	std::unique_ptr<VerticesRenderer> m_verticesRenderer;
	std::unique_ptr<EdgesRenderer> m_edgesRenderer;
	std::unique_ptr<FacesRenderer> m_facesRenderer;

	std::unique_ptr<AccelerationStructureBVH> m_accelerationStructure;
};
