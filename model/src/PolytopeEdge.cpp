#include "PolytopeEdge.h"
#include "Cylinder.h"

#include <algorithm>
#include <cmath>
#include <limits>

PolytopeEdge::PolytopeEdge(std::vector<Point> vertices_a, std::vector<Point> vertices_b, float radius)
	: ImplicitFunctionSet<3>(std::make_unique<Cylinder>(
		vertices_a.empty() ? Point{} : vertices_a.front(),
		vertices_b.empty() ? Point{} : vertices_b.front(),
		radius)),
	m_vertices_a(std::move(vertices_a)),
	m_vertices_b(std::move(vertices_b)),
	m_radius(radius)
{
	m_primitiveCount = static_cast<int>(std::min(m_vertices_a.size(), m_vertices_b.size()));
	initializePolytopeEdgeBuffer();
}

PolytopeEdge::~PolytopeEdge()
{
	if (m_polytopeEdgeBuffer == 0)
		return;

	QOpenGLContext* context = QOpenGLContext::currentContext();
	if (context)
	{
		QOpenGLExtraFunctions* gl = context->extraFunctions();
		gl->initializeOpenGLFunctions();
		gl->glDeleteBuffers(1, &m_polytopeEdgeBuffer);
	}
}

void PolytopeEdge::initializePolytopeEdgeBuffer() const
{
	QOpenGLContext* context = QOpenGLContext::currentContext();
	if (!context)
		return;

	QOpenGLExtraFunctions* gl = context->extraFunctions();
	gl->initializeOpenGLFunctions();

	if (m_polytopeEdgeBuffer == 0)
	{
		std::vector<float> edgeData;
		edgeData.reserve(static_cast<size_t>(m_primitiveCount) * 8);

		for (int i = 0; i < m_primitiveCount; ++i)
		{
			const Point& a = m_vertices_a[static_cast<size_t>(i)];
			const Point& b = m_vertices_b[static_cast<size_t>(i)];
			edgeData.insert(edgeData.end(), { a[0], a[1], a[2], 0.0f, b[0], b[1], b[2], 0.0f });
		}

		gl->glGenBuffers(1, &m_polytopeEdgeBuffer);
		gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_polytopeEdgeBuffer);
		gl->glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(edgeData.size() * sizeof(float)),
			edgeData.data(),
			GL_STATIC_DRAW);
	}

	gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_polytopeEdgeBuffer);
	gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

float PolytopeEdge::evaluateSDF(const Point& p) const
{
	float distance = std::numeric_limits<float>::max();
	for (int i = 0; i < m_primitiveCount; ++i)
	{
		const Point& a = m_vertices_a[static_cast<size_t>(i)];
		const Point& b = m_vertices_b[static_cast<size_t>(i)];
		const float vx = b[0] - a[0];
		const float vy = b[1] - a[1];
		const float vz = b[2] - a[2];
		const float wx = p[0] - a[0];
		const float wy = p[1] - a[1];
		const float wz = p[2] - a[2];
		const float lengthSquared = vx * vx + vy * vy + vz * vz;
		const float projection = vx * wx + vy * wy + vz * wz;
		const float t = lengthSquared > 1e-8f
			? std::clamp(projection / lengthSquared, 0.0f, 1.0f)
			: 0.0f;
		const float dx = p[0] - (a[0] + t * vx);
		const float dy = p[1] - (a[1] + t * vy);
		const float dz = p[2] - (a[2] + t * vz);
		distance = std::min(distance, std::sqrt(dx * dx + dy * dy + dz * dz) - m_radius);
	}
	return distance;
}

void PolytopeEdge::setGLUniforms(QOpenGLShaderProgram& program, int indexCurrentPrimitive) const
{
	program.setUniformValue("u_currentSphereIndex", indexCurrentPrimitive);
	program.setUniformValue("u_isEdge", true);

	dynamic_cast<Cylinder*>(m_refPrimitive.get())->setRadius(m_radius);

	m_refPrimitive->setGLUniforms(program);
}

const std::string PolytopeEdge::getVertexShader() const
{
	return PolytopeShaderBuilder::buildVertexShader();
}

const std::string PolytopeEdge::getFragmentShader() const
{
	return PolytopeShaderBuilder::buildFragmentShader();
}