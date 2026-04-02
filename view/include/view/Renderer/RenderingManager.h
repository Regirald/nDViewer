#pragma once
#include <memory>
#include <vector>
#include "Renderer.h"
#include "Renderer/RenderContext.h"
#include "model/Mesh.h"

template <size_t Dim>
class Mesh;

class RenderingManager
{
public:
	void renderScene();
	void setMesh(Mesh<2>* mesh);
	void setBackground();
	void setExampleTriangle();
	void setViewport(int width, int height);
	void setCamera(const ndv::math::Mat4& view, const ndv::math::Mat4& projection, const ndv::math::Vec3& cameraPosition);
	void setTime(float timeSeconds);

	ndv::RenderContext& context();

private:
    std::vector<std::unique_ptr<Renderer>> renderContainer;
	ndv::RenderContext m_context;
};