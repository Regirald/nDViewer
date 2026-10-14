#include "PolytopeNodeLightRenderer.h"

#include <QOpenGLContext>
#include "core/Pipeline/TransformPipeline.h"

PolytopeNodeLightRenderer::~PolytopeNodeLightRenderer()
{
	// Nothing to free right now; just clear GL function pointer
	m_gl = nullptr;
	m_initialized = false;
}

void PolytopeNodeLightRenderer::initializeGLResources()
{
	if (m_initialized)
		return;

	QOpenGLContext* ctx = QOpenGLContext::currentContext();
	if (ctx)
	{
		m_gl = ctx->functions();
		m_initialized = (m_gl != nullptr);
	}
}

void PolytopeNodeLightRenderer::render(const ndv::RenderContext& context)
{
	if (!m_initialized)
		initializeGLResources();

	if (!m_initialized)
		return;

	// Convert node position (world space) to clip/NDC coordinates
	const QVector4D worldPos((float)m_node.node[0], (float)m_node.node[1], (float)m_node.node[2], 1.0f);
	const QVector4D viewPos = ndv::pipeline::TransformPipeline::WorldToView(worldPos, context.view);
	const QVector4D clipPos = ndv::pipeline::TransformPipeline::ViewToClip(viewPos, context.projection);
	const QVector3D ndcPos = ndv::pipeline::TransformPipeline::ClipToNDC(clipPos);

	// Simple immediate-mode point rendering in NDC coordinates
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw a simple point. Size is constant; radius in the model is ignored for this simple display.
	glPointSize(8.0f);
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_POINTS);
	glVertex3f(ndcPos.x(), ndcPos.y(), ndcPos.z());
	glEnd();
}
