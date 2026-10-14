#pragma once
#include <QOpenGLFunctions>

#include "Renderer/Renderer.h"
#include "model/PolytopeEdge.h"

class PolytopeEdgeLightRenderer : public Renderer
{
public:
	PolytopeEdgeLightRenderer(const std::pair<PolytopeEdge::Point, PolytopeEdge::Point>& seg, float radius)
		: m_a(seg.first), m_b(seg.second), m_radius(radius)
	{
	}
	~PolytopeEdgeLightRenderer() override;

	void render(const ndv::RenderContext& context) override;

	void initializeGLResources();
private:
	bool m_initialized = false;
	QOpenGLFunctions* m_gl = nullptr;

	PolytopeEdge::Point m_a;
	PolytopeEdge::Point m_b;
	float m_radius = 0.0f;
};
