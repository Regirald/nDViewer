#include "RenderingManager.h"
#include "RenderMesh.h"
#include "model/Mesh.h"

void RendingManager::setMesh(Mesh<3>* mesh)
{
	Render* render = new RenderMesh<3>(*mesh);
	renderContainer.push_back(render);
}


void RendingManager::renderScene()
{
	for (Render* render : renderContainer)
	{
		render->render();
	}
}