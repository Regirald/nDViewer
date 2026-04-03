#pragma once

#include <Windows.h>
#include <gl/GL.h>
#include <QOpenGLFunctions>

#ifdef render
#undef render
#endif

#include "Renderer/Renderer.h"
#include "Renderer/RenderContext.h"

// Renders a small sphere at world-space position context.targetPosition
class RenderTargetSphere : public Renderer
{
public:
    RenderTargetSphere();
    ~RenderTargetSphere() override;

    void render(const ndv::RenderContext& context) override;

private:
    void initializeGLResources();

    bool m_initialized = false;
    QOpenGLFunctions* m_gl = nullptr;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    int m_indexCount = 0;
};
