#pragma once

#include <QMatrix4x4>
#include <QVector3D>

namespace ndv
{
    struct RenderContext
    {
        QMatrix4x4 view;
        QMatrix4x4 projection;
        QVector3D cameraPosition{ 0.0f, 0.0f, 0.0f };
        QVector3D targetPosition{ 0.0f, 0.0f, 0.0f };
        QVector3D lightningDirection{ 1.0f, 1.0f, 0.0f };



        int viewportWidth = 1;
        int viewportHeight = 1;

        float time = 0.0f;
        // When true, full shaded faces are rendered. When false, only vertices/edges are drawn for
        // a cheaper rendering mode.
        bool fullShadedRendering = true;
    };
}
