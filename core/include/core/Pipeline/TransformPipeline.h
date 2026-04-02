#pragma once

#include "core/Math/Mat4.h"
#include "core/Math/Vec3.h"
#include "core/Math/Vec4.h"

namespace ndv::pipeline
{
    struct TransformPipeline
    {
        // Object Space -> World Space: worldPos = Model * localPos
        static math::Vec4 ObjectToWorld(const math::Vec4& localPos, const math::Mat4& model)
        {
            return model * localPos;
        }

        // World Space -> View Space: viewPos = View * worldPos
        // Note: View is the inverse of the camera's world transform.
        static math::Vec4 WorldToView(const math::Vec4& worldPos, const math::Mat4& view)
        {
            return view * worldPos;
        }

        // View Space -> Clip Space: clipPos = Projection * viewPos
        static math::Vec4 ViewToClip(const math::Vec4& viewPos, const math::Mat4& projection)
        {
            return projection * viewPos;
        }

        // Clip Space -> NDC: ndc = clip.xyz / clip.w
        // Perspective divide is required to undo homogeneous projection.
        static math::Vec3 ClipToNDC(const math::Vec4& clipPos)
        {
            if (clipPos.w == 0.0f)
            {
                return {};
            }
            return { clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w };
        }

        // NDC -> Screen: map from [-1,1] to pixel coordinates.
        // Common pitfall: Y origin differs between OpenGL (bottom-left) and UI toolkits (top-left).
        static math::Vec2 NDCToScreen(const math::Vec3& ndcPos, float width, float height)
        {
            const float x = (ndcPos.x + 1.0f) * 0.5f * width;
            const float y = (1.0f - (ndcPos.y + 1.0f) * 0.5f) * height;
            return { x, y };
        }
    };
}
