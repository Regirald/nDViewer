#pragma once

#include "core/Math/Mat4.h"
#include "core/Math/Vec3.h"

namespace ndv
{
    struct RenderContext
    {
        math::Mat4 view = math::Mat4::identity();
        math::Mat4 projection = math::Mat4::identity();
        math::Vec3 cameraPosition{ 0.0f, 0.0f, 0.0f };

        int viewportWidth = 1;
        int viewportHeight = 1;

        float time = 0.0f;
    };
}
