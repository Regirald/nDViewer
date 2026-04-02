#pragma once

#include "Renderer/Renderer.h"
#include "core/Transform/Transform.h"
#include "core/Pipeline/TransformPipeline.h"

#include <iostream>

namespace ndv
{
    class ExampleTriangleRenderer : public Renderer
    {
    public:
        ExampleTriangleRenderer()
        {
            m_transform.position = { 0.0f, 0.0f, 0.0f };
            m_transform.rotationRadians = { 0.0f, 0.0f, 0.0f };
            m_transform.scale = { 1.0f, 1.0f, 1.0f };

        }

        void render(const RenderContext& context) override
        {
            if (m_logged)
            {
                return;
            }
            m_logged = true;

            const math::Vec4 localPos{ 0.5f, 0.5f, 0.0f, 1.0f };
            const math::Mat4 model = m_transform.modelMatrix();
            const math::Mat4 view = context.view;
            const math::Mat4 projection = context.projection;

            // Full pipeline: clipPos = Projection * View * Model * localPos
            const math::Vec4 worldPos = pipeline::TransformPipeline::ObjectToWorld(localPos, model);
            const math::Vec4 viewPos = pipeline::TransformPipeline::WorldToView(worldPos, view);
            const math::Vec4 clipPos = pipeline::TransformPipeline::ViewToClip(viewPos, projection);
            const math::Vec3 ndcPos = pipeline::TransformPipeline::ClipToNDC(clipPos);
            const math::Vec2 screenPos = pipeline::TransformPipeline::NDCToScreen(
                ndcPos,
                static_cast<float>(context.viewportWidth),
                static_cast<float>(context.viewportHeight));

            std::cout << "ExampleTriangleRenderer debug pipeline:\n";
            std::cout << "  Object (local): " << localPos << "\n";
            std::cout << "  World: " << worldPos << "\n";
            std::cout << "  View: " << viewPos << "\n";
            std::cout << "  Clip: " << clipPos << "\n";
            std::cout << "  NDC: " << ndcPos << "\n";
            std::cout << "  Screen: " << screenPos << "\n";
        }

    private:
        Transform m_transform;
        bool m_logged = false;
    };
}
