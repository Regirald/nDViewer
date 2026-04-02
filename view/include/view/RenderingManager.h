#pragma once
#include <vector>
#include "Render.h"
#include "model/Mesh.h"

template <size_t Dim>
class Mesh;

class RenderingManager
{
public:
	void renderScene();
	void setMesh(Mesh<2>* mesh);
	void setBackground();

	std::vector<Render*> renderContainer;
};