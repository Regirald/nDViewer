#include "VerticeSet.h"
#include "Sphere.h"

VerticeSet::VerticeSet(std::vector<Point> vertices, float radius)
	: ImplicitFunctionSet<3>(std::make_unique<Sphere>()),
	vertices(vertices),
	radius(radius)
{


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
	const Point& vertex = vertices[indexCurrentPrimitive];
	auto sphere = dynamic_cast<Sphere*>(m_refPrimitive.get());

	sphere->m_boundsMin = QVector3D(vertex[0] - radius, vertex[1] - radius, vertex[2] - radius);
	sphere->m_boundsMax = QVector3D(vertex[0] + radius, vertex[1] + radius, vertex[2] + radius);
	sphere->m_center = vertex;
	sphere->m_radius = radius;

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

