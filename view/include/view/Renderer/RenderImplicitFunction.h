#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

#include "Renderer/Renderer.h"
#include "model/ImplicitFunction.h"

template <size_t Dim>
class RenderImplicitFunction : public Renderer
{
public:
	explicit RenderImplicitFunction(std::unique_ptr<ImplicitFunction<Dim>> implicitFunction);
	~RenderImplicitFunction();

	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

	std::unique_ptr<ImplicitFunction<Dim>> m_implicitFunction;
	QOpenGLShaderProgram m_program;
	QOpenGLExtraFunctions* gl = nullptr;
	bool m_initialized = false;
	bool m_isTransparent = false;

	GLuint m_fullscreenVAO = 0;
    GLuint m_fullscreenVBO = 0;
	QVector3D m_boundsMin = QVector3D(-2, -2, -2);
	QVector3D m_boundsMax = QVector3D(2, 2, 2);


};