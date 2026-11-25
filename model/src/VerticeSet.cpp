#include "VerticeSet.h"

template <size_t Dim>
VerticeSet<Dim>::VerticeSet(
	std::vector<Point> vertices,
	float radius)
	: vertices(vertices),
	  radius(radius)
{

	QVector3D rootBoundsMin;
	QVector3D rootBoundsMax;
	for (auto vertex : vertices)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			rootBoundsMin[i] = std::min(rootBoundsMin[i], vertex[i] - radius);
			rootBoundsMax[i] = std::max(rootBoundsMax[i], vertex[i] + radius);
		}

	}

	ImplicitFunction<Dim>::m_boundsMin = rootBoundsMin;
	ImplicitFunction<Dim>::m_boundsMax = rootBoundsMax;

	m_dstFunctionName = "sampleVertexBruteForce";
	m_uniformsNames.push_back("u_vertexPosition");
	m_uniformsNames.push_back("u_radius");
}


template <size_t Dim>
float VerticeSet<Dim>::evaluateSDF(const Point& p) const
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

template <size_t Dim>
void VerticeSet<Dim>::setGLUniforms(QOpenGLShaderProgram& program) const
{
	u_radius = ctx.makeUniformUnique("u_radius", "float");
	u_center = ctx.makeUniformUnique("u_center", "vec3");
	ImplicitFunction<Dim>::setGLUniforms(program);



	program.setUniformValue(m_uniformsNames.back().c_str(), u_vertexPosition);
	program.setUniformValue(m_uniformsNames.back().c_str()), radius);
	program.setUniformValue("u_bvhVertexCount", static_cast<int>(vertices.size()));

}

template <size_t Dim>
const std::string VerticeSet<Dim>::buildSDF() const
{
	std::string functionSDF = "float " + m_dstFunctionName + "(vec3 p)\n";
	functionSDF.append("    return length(p - " + u_center + ") - " + u_radius + ";");
	functionSDF.append("}");

	return functionSDF;
}


template <size_t Dim>
const std::string VerticeSet<Dim>::getFragmentShader() const
{
	const std::string dstFunction = buildDistanceFunction();
	return PolytopeShaderBuilder::buildFragmentShader(ctx, dstFunction, PolytopeShaderBuilder::PolytopeShaderStyle::Node, Dim);
}



template class VerticeSet<2>;
template class VerticeSet<3>;
template class VerticeSet<4>;
