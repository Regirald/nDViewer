#pragma once

#include "Renderer/Renderer.h"
#include "core/Transform/Transform.h"
#include "core/Pipeline/TransformPipeline.h"

#include <Windows.h>
#include <gl/GL.h>

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
            const math::Vec4 localPositions[] =
            {
                { -0.5f, -0.5f, 0.0f, 1.0f },
                {  0.5f, -0.5f, 0.0f, 1.0f },
                {  0.0f,  0.5f, 0.0f, 1.0f }
            };
            const math::Mat4 model = m_transform.modelMatrix();
            const math::Mat4 view = context.view;
            const math::Mat4 projection = context.projection;

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glColor3f(1.0f, 0.4f, 0.2f);
            glBegin(GL_TRIANGLES);
            for (const auto& localPos : localPositions)
            {
                const math::Vec4 worldPos = pipeline::TransformPipeline::ObjectToWorld(localPos, model);
                const math::Vec4 viewPos = pipeline::TransformPipeline::WorldToView(worldPos, view);
                const math::Vec4 clipPos = pipeline::TransformPipeline::ViewToClip(viewPos, projection);
                const math::Vec3 ndcPos = pipeline::TransformPipeline::ClipToNDC(clipPos);
                glVertex3f(ndcPos.x, ndcPos.y, ndcPos.z);
            }
            glEnd();
        }

    private:
        Transform m_transform;
    };
}
