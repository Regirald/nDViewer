#pragma once

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>



#include "Renderer/Renderer.h"
#include "model/Mesh.h"

template <size_t Dim>
class RenderMesh : public Renderer
{
public:
	RenderMesh(Mesh<Dim>* mesh);
	~RenderMesh();

 void initializeGLResources();
 void render(const ndv::RenderContext& context) override;


    GLuint vbo = 0;
    GLuint ebo = 0;
	GLuint vao = 0;
	int m_indexCount = 0;
	bool m_initialized = false;
	QOpenGLExtraFunctions* gl = nullptr;
	QOpenGLShaderProgram m_program;
	Mesh<Dim>* m_mesh;
};
