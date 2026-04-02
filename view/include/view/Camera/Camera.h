#pragma once

#include "core/Math/Mat4.h"

namespace ndv
{
    class Camera
    {
    public:
        math::Vec3 position{ 0.0f, 0.0f, 3.0f };
        math::Vec3 target{ 0.0f, 0.0f, 0.0f };
        math::Vec3 up{ 0.0f, 1.0f, 0.0f };
        float fovYRadians = 60.0f * 3.1415926535f / 180.0f;
        float aspectRatio = 1.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;

        math::Mat4 viewMatrix() const;
        math::Mat4 projectionMatrix() const;
    };
}
