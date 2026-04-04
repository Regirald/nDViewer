#pragma once

#include <QMatrix4x4>
#include <QVector3D>
#include <QtMath>

namespace ndv
{
    struct Transform
    {
        QVector3D position{ 0.0f, 0.0f, 0.0f };
        QVector3D rotationRadians{ 0.0f, 0.0f, 0.0f };
        QVector3D scale{ 1.0f, 1.0f, 1.0f };

        QMatrix4x4 modelMatrix() const
        {
            QMatrix4x4 model;
            model.setToIdentity();
            model.translate(position);
            model.rotate(qRadiansToDegrees(rotationRadians.z()), 0.0f, 0.0f, 1.0f);
            model.rotate(qRadiansToDegrees(rotationRadians.y()), 0.0f, 1.0f, 0.0f);
            model.rotate(qRadiansToDegrees(rotationRadians.x()), 1.0f, 0.0f, 0.0f);
            model.scale(scale);
            return model;
        }
    };
}
