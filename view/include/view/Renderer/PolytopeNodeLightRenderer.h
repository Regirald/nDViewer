#pragma once
#include <QOpenGLFunctions>

#include "Renderer/Renderer.h"
#include "model/PolytopeNodeLight.h"

class PolytopeNodeLightRenderer : public Renderer
{
public:
	PolytopeNodeLightRenderer(const PolytopeNodeLight& node)
		: m_node(node)
	{
	}
    ~PolytopeNodeLightRenderer() override;

    void render(const ndv::RenderContext& context) override;

    void initializeGLResources();
private:

    bool m_initialized = false;
    QOpenGLFunctions* m_gl = nullptr;

	PolytopeNodeLight m_node;
};