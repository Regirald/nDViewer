#pragma once
#define NOMINMAX
#include <Windows.h>
#include <gl/GL.h>

#include "Renderer/RenderContext.h"

class Renderer
{
public:
    virtual ~Renderer() = default;
    virtual void render(const ndv::RenderContext& context) = 0;
};
