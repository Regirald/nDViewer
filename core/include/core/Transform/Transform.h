#pragma once

#include "core/Math/Mat4.h"

namespace ndv
{
    struct Transform
    {
        math::Vec3 position{ 0.0f, 0.0f, 0.0f };
        math::Vec3 rotationRadians{ 0.0f, 0.0f, 0.0f };
        math::Vec3 scale{ 1.0f, 1.0f, 1.0f };

        math::Mat4 modelMatrix() const
        {
            const math::Mat4 translate = math::Mat4::translation(position);
            const math::Mat4 rotateX = math::Mat4::rotationX(rotationRadians.x);
            const math::Mat4 rotateY = math::Mat4::rotationY(rotationRadians.y);
            const math::Mat4 rotateZ = math::Mat4::rotationZ(rotationRadians.z);
            const math::Mat4 scaleMatrix = math::Mat4::scale(scale);

            // Model = T * Rz * Ry * Rx * S (column-major, column vectors).
            return translate * rotateZ * rotateY * rotateX * scaleMatrix;
        }
    };
}
