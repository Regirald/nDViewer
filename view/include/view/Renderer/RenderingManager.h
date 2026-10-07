#pragma once
#include <memory>
#include <vector>
#include "renderer.h"
#include "RenderContext.h"
#include "model/Mesh.h"
#include "RenderMesh.h"
#include "RenderImplicitFunction.h"
#include "RenderPolytope.h"

template <size_t Dim>
class Mesh;

class RenderingManager
{
public:
	void renderScene();

template <size_t Dim>
	inline void setMesh(Mesh<Dim>* mesh)
	{
		renderContainer.push_back(std::make_unique<RenderMesh<Dim>>(mesh));
	}
template <size_t Dim>
	inline void setImplicitFunction(ImplicitFunction<Dim>* implicitFunction)
	{
		renderContainer.push_back(std::make_unique<RenderImplicitFunction<Dim>>(implicitFunction));
	}

	void setPolytope(PolytopeGeometry3D* polytope);

	void setBackground();
	void setExampleTriangle();
	void setViewport(int width, int height);
  void setCamera(const QMatrix4x4& view,
				  const QMatrix4x4& projection,
				  const QVector3D& cameraPosition,
				  const QVector3D& cameraTarget);
	void setTime(float timeSeconds);

	ndv::RenderContext& context();

private:
    std::vector<std::unique_ptr<Renderer>> renderContainer;
	ndv::RenderContext m_context;
};