#include "VerticeSet.h"
#include "Sphere.h"

VerticeSet::VerticeSet(std::vector<Point> vertices, float radius)
	: ImplicitFunctionSet<3>(std::make_unique<Sphere>()),
	vertices(vertices),
	radius(radius)
{
	m_primitiveCount = static_cast<int>(this->vertices.size());
	initializeVerticeSetBuffer();
}

VerticeSet::~VerticeSet()
{
	if (m_verticeSetBuffer == 0)
		return;

	QOpenGLContext* context = QOpenGLContext::currentContext();
	if (context)
	{
		QOpenGLExtraFunctions* gl = context->extraFunctions();
		gl->initializeOpenGLFunctions();
		gl->glDeleteBuffers(1, &m_verticeSetBuffer);
	}
}

void VerticeSet::initializeVerticeSetBuffer() const
{
	QOpenGLContext* context = QOpenGLContext::currentContext();
	if (!context)
		return;

	QOpenGLExtraFunctions* gl = context->extraFunctions();
	gl->initializeOpenGLFunctions();

	if (m_verticeSetBuffer == 0)
	{
		std::vector<float> vertexData;
		vertexData.reserve(vertices.size() * 4);

		for (const Point& vertex : vertices)
		{
			vertexData.push_back(vertex[0]);
			vertexData.push_back(vertex[1]);
			vertexData.push_back(vertex[2]);
			vertexData.push_back(0.0f);
		}

		gl->glGenBuffers(1, &m_verticeSetBuffer);
		gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_verticeSetBuffer);
		gl->glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(vertexData.size() * sizeof(float)),
			vertexData.data(),
			GL_STATIC_DRAW);
	}

	gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_verticeSetBuffer);
	gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


float VerticeSet::evaluateSDF(const Point& p) const
{
	float distance = std::numeric_limits<float>::max();
	for (const Point& vertex : vertices)
	{
		const float dx = p[0] - vertex[0];
		const float dy = p[1] - vertex[1];
		const float dz = p[2] - vertex[2];
		distance = std::min(distance, std::sqrt(dx * dx + dy * dy + dz * dz) - radius);
	}
	return distance;
}


void VerticeSet::setGLUniforms(QOpenGLShaderProgram& program, int indexCurrentPrimitive) const
{
	program.setUniformValue("u_currentSphereIndex", indexCurrentPrimitive);

	dynamic_cast<Sphere*>(m_refPrimitive.get())->m_radius = radius;

	m_refPrimitive->setGLUniforms(program);

}

const std::string VerticeSet::getVertexShader() const
{
	return PolytopeShaderBuilder::buildVertexShader();
}

const std::string VerticeSet::getFragmentShader() const
{
	return PolytopeShaderBuilder::buildFragmentShader();
}

