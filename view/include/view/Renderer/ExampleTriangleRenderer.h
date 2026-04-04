#pragma once

#include "Renderer/Renderer.h"
#include "core/Transform/Transform.h"
#include "core/Pipeline/TransformPipeline.h"

#include <QVector4D>

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
            const QVector4D localPositions[] =
            {
                { -0.5f, -0.5f, 0.0f, 1.0f },
                {  0.5f, -0.5f, 0.0f, 1.0f },
                {  0.0f,  0.5f, 0.0f, 1.0f }
            };
            const QMatrix4x4 model = m_transform.modelMatrix();
            const QMatrix4x4 view = context.view;
            const QMatrix4x4 projection = context.projection;

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glColor3f(1.0f, 0.4f, 0.2f);
            glBegin(GL_TRIANGLES);
            for (const auto& localPos : localPositions)
            {
                const QVector4D worldPos = pipeline::TransformPipeline::ObjectToWorld(localPos, model);
                const QVector4D viewPos = pipeline::TransformPipeline::WorldToView(worldPos, view);
                const QVector4D clipPos = pipeline::TransformPipeline::ViewToClip(viewPos, projection);
                const QVector3D ndcPos = pipeline::TransformPipeline::ClipToNDC(clipPos);
                glVertex3f(ndcPos.x(), ndcPos.y(), ndcPos.z());
            }
            glEnd();
        }

    private:
        Transform m_transform;
    };
}
