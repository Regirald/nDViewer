#include <memory>
#include "RenderingManager.h"
#include "RenderBackground.h"
#include "Renderer/ExampleTriangleRenderer.h"
#include "Renderer/RenderTargetSphere.h"


void RenderingManager::setBackground()
{
    renderContainer.push_back(std::make_unique<RenderBackground>());
	// also render a small sphere to mark the camera target/position
	renderContainer.push_back(std::make_unique<RenderTargetSphere>());
}

void RenderingManager::setExampleTriangle()
{
    renderContainer.push_back(std::make_unique<ndv::ExampleTriangleRenderer>());
}

void RenderingManager::setViewport(int width, int height)
{
    m_context.viewportWidth = width > 0 ? width : 1;
	m_context.viewportHeight = height > 0 ? height : 1;
}

void RenderingManager::setCamera(const QMatrix4x4& view,
	const QMatrix4x4& projection,
	const QVector3D& cameraPosition,
	const QVector3D& cameraTarget)
{
	m_context.view = view;
	m_context.projection = projection;
	m_context.cameraPosition = cameraPosition;
   m_context.targetPosition = cameraTarget;
}

void RenderingManager::setTime(float timeSeconds)
{
	m_context.time = timeSeconds;
}

ndv::RenderContext& RenderingManager::context()
{
	return m_context;
}

void RenderingManager::setPolytope(PolytopeGeometry3D* polytope)
{
	std::unique_ptr<Renderer>* renderer = getPolytopeRenderer();
	if (renderer != nullptr)
	{
		*renderer = std::make_unique<RenderPolytope>(polytope);
	}
	else
	{
		renderContainer.push_back(std::make_unique<RenderPolytope>(polytope));
	}
}


std::unique_ptr<Renderer>* RenderingManager::getPolytopeRenderer()
{
	for (auto& renderer : renderContainer)
	{
		RenderPolytope* polytopeRenderer = dynamic_cast<RenderPolytope*>(renderer.get());
		if (polytopeRenderer != nullptr)
		{
			return &renderer;
		}
	}
	return nullptr;
}

void RenderingManager::renderScene()
{
    for (const auto& render : renderContainer)
	{
		render->render(m_context);
	}
}
