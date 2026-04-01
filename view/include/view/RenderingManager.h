#pragma once
#include <vector>
#include "Render.h"

class Mesh<3>
class RendingManager
{
public:
	void renderScene();
	void setMesh(Mesh<3>* mesh);

	std::vector<Render*> renderContainer;

};