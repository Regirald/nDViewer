#include "RenderingManager.h"
#include "RenderMesh.h"
#include "RenderBackground.h"
#include "Renderer/ExampleTriangleRenderer.h"


void RenderingManager::setMesh(Mesh<2>* mesh)
{
    Renderer* render = new RenderMesh<2>(mesh);
	renderContainer.push_back(render);
}

void RenderingManager::setBackground()
{
    Renderer* render = new RenderBackground();
	renderContainer.push_back(render);
}

void RenderingManager::setExampleTriangle()
{
    Renderer* render = new ndv::ExampleTriangleRenderer();
	renderContainer.push_back(render);
}

void RenderingManager::setViewport(int width, int height)
{
    m_context.viewportWidth = width > 0 ? width : 1;
	m_context.viewportHeight = height > 0 ? height : 1;
}

void RenderingManager::setCamera(const ndv::math::Mat4& view, const ndv::math::Mat4& projection, const ndv::math::Vec3& cameraPosition)
{
	m_context.view = view;
	m_context.projection = projection;
	m_context.cameraPosition = cameraPosition;
}

void RenderingManager::setTime(float timeSeconds)
{
	m_context.time = timeSeconds;
}

ndv::RenderContext& RenderingManager::context()
{
	return m_context;
}


void RenderingManager::renderScene()
{
    for (Renderer* render : renderContainer)
	{
       render->render(m_context);
	}
}
