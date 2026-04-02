#pragma once
#include "Renderer/Renderer.h"

class RenderBackground : public Renderer
{
public:
 void render(const ndv::RenderContext& context) override;

};