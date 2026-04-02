#include "Camera/Camera.h"

namespace ndv
{
    math::Mat4 Camera::viewMatrix() const
    {
        // The view matrix moves the world into camera space (inverse of the camera transform).
        return math::Mat4::lookAt(position, target, up);
    }

    math::Mat4 Camera::projectionMatrix() const
    {
        return math::Mat4::perspective(fovYRadians, aspectRatio, nearPlane, farPlane);
    }
}
