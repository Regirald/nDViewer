#include "PolytopeEdgeLightRenderer.h"

#include <QOpenGLContext>
#include "core/Pipeline/TransformPipeline.h"

PolytopeEdgeLightRenderer::~PolytopeEdgeLightRenderer()
{
	// Clear GL function pointer
	m_gl = nullptr;
	m_initialized = false;
}

void PolytopeEdgeLightRenderer::initializeGLResources()
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

void PolytopeEdgeLightRenderer::render(const ndv::RenderContext& context)
{
	if (!m_initialized)
		initializeGLResources();

	if (!m_initialized)
		return;

	// Convert endpoints (world space) to clip/NDC coordinates
	const QVector4D worldA((float)m_a[0], (float)m_a[1], (float)m_a[2], 1.0f);
	const QVector4D worldB((float)m_b[0], (float)m_b[1], (float)m_b[2], 1.0f);

	const QVector4D viewA = ndv::pipeline::TransformPipeline::WorldToView(worldA, context.view);
	const QVector4D viewB = ndv::pipeline::TransformPipeline::WorldToView(worldB, context.view);
	const QVector4D clipA = ndv::pipeline::TransformPipeline::ViewToClip(viewA, context.projection);
	const QVector4D clipB = ndv::pipeline::TransformPipeline::ViewToClip(viewB, context.projection);
	const QVector3D ndcA = ndv::pipeline::TransformPipeline::ClipToNDC(clipA);
	const QVector3D ndcB = ndv::pipeline::TransformPipeline::ClipToNDC(clipB);

	// Simple immediate-mode line rendering in NDC coordinates
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(2.0f);
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(ndcA.x(), ndcA.y(), ndcA.z());
	glVertex3f(ndcB.x(), ndcB.y(), ndcB.z());
	glEnd();
}
