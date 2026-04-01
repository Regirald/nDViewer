#pragma once
#include <vector>
#include <windows.h>
#include <gl/GL.h>
#include <windowsx.h> 

class Mesh;

class Render
{
public:
	Render();
	virtual void render() {};
};

