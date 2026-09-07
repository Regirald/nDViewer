#pragma once

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

#include "Renderer/Renderer.h"
#include "model/GeometryFromTopology.h"

template <size_t Dim>
class RenderPolytope : public Renderer
{
public:
	RenderPolytope(PolytopeGeometry<Dim>* polytope);
	~RenderPolytope();

	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

	GLuint vbo = 0;
	GLuint eboEdges = 0;
	GLuint eboFaces = 0;
	GLuint vao = 0;
	int m_indexCount = 0; // edge index count
	int m_faceIndexCount = 0;
	bool m_initialized = false;
	QOpenGLExtraFunctions* gl = nullptr;
	QOpenGLShaderProgram m_program;
	PolytopeGeometry<Dim>* m_polytope;
};
