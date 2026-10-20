#pragma once
#include <QOpenGLExtraFunctions>

#include "Renderer/Renderer.h"
#include "model/PolytopeEdge.h"

class PolytopeEdgeLightRenderer : public Renderer
{
public:
	PolytopeEdgeLightRenderer(const std::pair<PolytopeEdge::Point, PolytopeEdge::Point>& seg)
		: m_a(seg.first), m_b(seg.second)
	{
	}
	~PolytopeEdgeLightRenderer() override;

	void render(const ndv::RenderContext& context) override;

	void initializeGLResources();
private:
	PolytopeEdge::Point m_a;
	PolytopeEdge::Point m_b;

	QOpenGLExtraFunctions* m_gl = nullptr;
	QOpenGLShaderProgram* m_program = nullptr;

	unsigned int m_vao = 0;
	unsigned int m_vbo = 0;

	bool m_initialized = false;
};
