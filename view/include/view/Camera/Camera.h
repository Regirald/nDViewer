#pragma once

#include <QVector3D>
#include <QMatrix4x4>
#include <QtMath> // For qSin, qCos, etc.

namespace ndv
{
    class Camera
    {
    public:
        QVector3D position{ 0.0f, 0.0f, 3.0f };
        QVector3D target{ 0.0f, 0.0f, 0.0f };
        QVector3D up{ 0.0f, 1.0f, 0.0f };
        float fovYRadians = 60.0f * 3.1415926535f / 180.0f;
        float aspectRatio = 1.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;

        QMatrix4x4 viewMatrix() const;
        QMatrix4x4 projectionMatrix() const;

        QVector3D right() const
        {
            return QVector3D::crossProduct(target - position, up).normalized();
        }

        void updatePositionFromYawPitch(float deltaYaw, float deltaPitch);
    };
}
