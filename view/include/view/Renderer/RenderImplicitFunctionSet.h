#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

#include "Renderer/Renderer.h"
#include "model/ImplicitFunctionSet.h"

template <size_t Dim>
class RenderImplicitFunctionSet : public Renderer
{
public:
	explicit RenderImplicitFunctionSet(std::unique_ptr<ImplicitFunctionSet<Dim>> implicitFunctionSet);
	~RenderImplicitFunctionSet();

	void initializeGLResources();
	void render(const ndv::RenderContext& context) override;

	std::unique_ptr<ImplicitFunctionSet<Dim>> m_implicitFunctionSet;

	QOpenGLShaderProgram m_program;
	QOpenGLExtraFunctions* gl = nullptr;

	bool m_initialized = false;
	bool m_isTransparent = false;

	GLuint m_fullscreenVAO = 0;
	GLuint m_fullscreenVBO = 0;

};