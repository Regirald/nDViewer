#include "RenderingManager.h"
#include "RenderMesh.h"

void RenderingManager::setMesh(Mesh<2>* mesh)
{
	Render* render = new RenderMesh<2>(mesh);
	renderContainer.push_back(render);
}


void RenderingManager::renderScene()
{
	for (Render* render : renderContainer)
	{
		render->render();
	}
}
