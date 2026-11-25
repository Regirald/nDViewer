//#include "VerticeSet.h"
//
//template <size_t Dim>
//VerticeSet<Dim>::VerticeSet(
//	std::vector<Point> vertices,
//	float radius)
//	: vertices(vertices),
//	  radius(radius)
//{
//	QVector3D rootBoundsMin(
//		std::numeric_limits<float>::max(),
//		std::numeric_limits<float>::max(),
//		std::numeric_limits<float>::max());
//	QVector3D rootBoundsMax(
//		std::numeric_limits<float>::lowest(),
//		std::numeric_limits<float>::lowest(),
//		std::numeric_limits<float>::lowest());
//	for (auto vertex : vertices)
//	{
//		for (size_t i = 0; i < 3; ++i)
//		{
//			rootBoundsMin[i] = std::min(rootBoundsMin[i], vertex[i] - radius);
//			rootBoundsMax[i] = std::max(rootBoundsMax[i], vertex[i] + radius);
//		}
//
//	}
//
//	ImplicitFunction<Dim>::m_boundsMin = rootBoundsMin;
//	ImplicitFunction<Dim>::m_boundsMax = rootBoundsMax;
//
//	m_dstFunctionName = "sampleSDF";
//	for (size_t i = 0; i < vertices.size(); ++i)
//		m_uniformsNames.push_back("u_vertexPosition_" + std::to_string(i));
//	m_uniformsNames.push_back("u_radius");
//}
//
//
//template <size_t Dim>
//float VerticeSet<Dim>::evaluateSDF(const Point& p) const
//{
//	float distance = std::numeric_limits<float>::max();
//	for (const Point& vertex : vertices)
//	{
//		const float dx = p[0] - vertex[0];
//		const float dy = p[1] - vertex[1];
//		const float dz = p[2] - vertex[2];
//		distance = std::min(distance, std::sqrt(dx * dx + dy * dy + dz * dz) - radius);
//	}
//	return distance;
//}
//
//template <size_t Dim>
//void VerticeSet<Dim>::setGLUniforms(QOpenGLShaderProgram& program) const
//{
//	ImplicitFunction<Dim>::setGLUniforms(program);
//
//	for (size_t i = 0; i < vertices.size(); ++i)
//	{
//		const Point& vertex = vertices[i];
//		program.setUniformValue(
//			m_uniformsNames[2 + i].c_str(),
//			QVector3D(vertex[0], vertex[1], vertex[2]));
//	}
//	program.setUniformValue(m_uniformsNames.back().c_str(), radius);
//
//}
//
//template <size_t Dim>
//const std::string VerticeSet<Dim>::buildSDF() const
//{
//	std::string functionSDF = "float " + m_dstFunctionName + "(vec3 p)\n{\n";
//	functionSDF += "    float d = 1e30;\n";
//	for (size_t i = 0; i < vertices.size(); ++i)
//		functionSDF += "    d = min(d, length(p - " + m_uniformsNames[2 + i] + ") - " + m_uniformsNames.back() + ");\n";
//	functionSDF += "    return d;\n}\n";
//
//	return functionSDF;
//}
//
//template <size_t Dim>
//const std::string VerticeSet<Dim>::buildUniforms() const
//{
//	std::string uniforms = ImplicitFunction<Dim>::buildUniforms();
//	for (size_t i = 0; i < vertices.size(); ++i)
//		uniforms += "uniform vec3 " + m_uniformsNames[2 + i] + ";\n";
//	uniforms += "uniform float " + m_uniformsNames.back() + ";\n";
//	return uniforms;
//}
//
//
//template <size_t Dim>
//const std::string VerticeSet<Dim>::getFragmentShader() const
//{
//	return PolytopeShaderBuilder::buildFragmentShader(buildUniforms(), buildSDF(), PolytopeShaderBuilder::PolytopeShaderStyle::Node);
//
//}
//
//
//
//template class VerticeSet<2>;
//template class VerticeSet<3>;
//template class VerticeSet<4>;
