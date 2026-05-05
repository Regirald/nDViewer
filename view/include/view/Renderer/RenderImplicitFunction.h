#pragma once
#include <Windows.h>
#include <gl/GL.h>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

#include "Renderer/Renderer.h"
#include "model/ImplicitFunction.h"

template <size_t Dim>
class RenderImplicitFunction : public Renderer
{
public:
	RenderImplicitFunction(ImplicitFunction<Dim>* implicitFunction);

	void initializeGLResources();
	void generateSDF();
	void uploadSDF(const std::vector<float>& data);
	void render(const ndv::RenderContext& context) override;

	ImplicitFunction<Dim>* m_implicitFunction;
	QOpenGLShaderProgram m_program;
	QOpenGLExtraFunctions* gl = nullptr;
	bool m_initialized = false;


	GLuint m_fullscreenVAO = 0;
    GLuint m_fullscreenVBO = 0;
	QVector3D m_boundsMin = QVector3D(-2, -2, -2);
	QVector3D m_boundsMax = QVector3D(2, 2, 2);


};