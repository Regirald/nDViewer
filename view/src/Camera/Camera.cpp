#include "Camera/Camera.h"


QMatrix4x4 ndv::Camera::viewMatrix() const
{
    // The view matrix moves the world into camera space (inverse of the camera transform).
    QMatrix4x4 mat;
    mat.lookAt(position, target, up);
    return mat;
}

QMatrix4x4 ndv::Camera::projectionMatrix() const
{
    QMatrix4x4 mat;
    mat.perspective(qRadiansToDegrees(fovYRadians), aspectRatio, nearPlane, farPlane);
    return mat;
}

void ndv::Camera::updatePositionFromYawPitch(float deltaYaw, float deltaPitch)
{
    // Vector from target to camera
    QVector3D offset = position - target;

    // Yaw rotation around world up
    QMatrix4x4 yawRotation;
    yawRotation.setToIdentity();
    yawRotation.rotate(qRadiansToDegrees(deltaYaw), up);
    offset = yawRotation.map(offset);

    // Recompute right vector in camera space after yaw
    QVector3D rightAxis = QVector3D::crossProduct(offset, up).normalized();

    // Pitch rotation around right axis
    QMatrix4x4 pitchRotation;
    pitchRotation.setToIdentity();
    pitchRotation.rotate(qRadiansToDegrees(deltaPitch), rightAxis);
    offset = pitchRotation.map(offset);

    // Update camera position
    position = target + offset;

    // Optional: recompute camera up vector to maintain orthogonality
    QVector3D newRight = QVector3D::crossProduct(target - position, up).normalized();
    up = QVector3D::crossProduct(newRight, target - position).normalized();
}

