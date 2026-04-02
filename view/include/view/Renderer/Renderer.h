#pragma once

#include "Renderer/RenderContext.h"

class Renderer
{
public:
    virtual ~Renderer() = default;
    virtual void render(const ndv::RenderContext& context) = 0;
};
