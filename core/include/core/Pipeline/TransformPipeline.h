#pragma once

#include <QMatrix4x4>
#include <QVector3D>
#include <QVector4D>


namespace ndv::pipeline
{
    struct TransformPipeline
    {
        // Object Space -> World Space: worldPos = Model * localPos
        static QVector4D ObjectToWorld(const QVector4D& localPos, const QMatrix4x4& model)
        {
            return model * localPos;
        }

        // World Space -> View Space: viewPos = View * worldPos
        // Note: View is the inverse of the camera's world transform.
        static QVector4D WorldToView(const QVector4D& worldPos, const QMatrix4x4& view)
        {
            return view * worldPos;
        }

        // View Space -> Clip Space: clipPos = Projection * viewPos
        static QVector4D ViewToClip(const QVector4D& viewPos, const QMatrix4x4& projection)
        {
            return projection * viewPos;
        }

        // Clip Space -> NDC: ndc = clip.xyz / clip.w
        // Perspective divide is required to undo homogeneous projection.
        static QVector3D ClipToNDC(const QVector4D& clipPos)
        {
            if (clipPos.w() == 0.0f)
            {
                return {};
            }
            return { clipPos.x() / clipPos.w(), clipPos.y() / clipPos.w(), clipPos.z() / clipPos.w() };
        }

        // NDC -> Screen: map from [-1,1] to pixel coordinates.
        // Common pitfall: Y origin differs between OpenGL (bottom-left) and UI toolkits (top-left).
        static QVector2D NDCToScreen(const QVector3D& ndcPos, float width, float height)
        {
            const float x = (ndcPos.x() + 1.0f) * 0.5f * width;
            const float y = (1.0f - (ndcPos.y() + 1.0f) * 0.5f) * height;
            return { x, y };
        }
    };
}
