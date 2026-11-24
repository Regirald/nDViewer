#include "VerticeSet.h"

#include <unordered_map>

namespace
{
template <size_t Dim>
std::unordered_map<const VerticeSet<Dim>*, GLuint>& verticeSetBuffers()
{
	static std::unordered_map<const VerticeSet<Dim>*, GLuint> buffers;
	return buffers;
}
}

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
std::string VerticeSet<Dim>::buildGLSL(GLSLContext& ctx) const
{
	const std::string vectorType = "vec" + std::to_string(Dim);
	const std::string dim = std::to_string(Dim);
	const std::string functionName = "sampleVertexBruteForce";

	ctx.uniforms.append("#extension GL_ARB_shader_storage_buffer_object : require\n");
	ctx.uniforms.append("layout(std430, binding = 0) buffer VerticeSetData { vec4 u_vertices[]; };\n");
	ctx.uniforms.append("uniform int u_bvhVertexCount;\n");
	ctx.uniforms.append("uniform float u_radius;\n");


	// BVH bounds distance.

	// Main BVH traversal.

	ctx.append("float " + functionName + "(vec3 p) {");

	ctx.append("\tfloat bestDistance = 1e10;");
	ctx.append("\tfor (int vertexIndex = 0; vertexIndex < u_bvhVertexCount; ++vertexIndex)");
	ctx.append("\t{");

	ctx.append("\t\tvec4 vertex = u_vertices[vertexIndex];");
	ctx.append("\t\tvec3 vertex3d = vertex.xyz / (1 - 0.3 * vertex.w);");
	ctx.append("\t\tfloat newDistance = length(p - vertex3d) - u_radius;");
	ctx.append("\t\tif(newDistance < bestDistance)");
	ctx.append("\t\t{");
	ctx.append("\t\t\tu_primitiveToutchedIndex = vertexIndex;");
	ctx.append("\t\t\tbestDistance = newDistance;");
	ctx.append("\t\t}");
	ctx.append("\t}");

	ctx.append("    return bestDistance;");
	ctx.append("}");


	ctx.append("float SDFonPrimitive(vec3 p) {");

	ctx.append("\tfloat bestDistance = 1e10;");
	ctx.append("\tint vertexIndex = u_primitiveToutchedIndex;");

	ctx.append("\tvec4 vertex = u_vertices[vertexIndex];");
	ctx.append("\tvec3 vertex3d = vertex.xyz / (1 - 0.3 * vertex.w);");
	ctx.append("\tbestDistance = min(bestDistance, length(p - vertex3d) - u_radius);");

	ctx.append("    return bestDistance;");
	ctx.append("}");

	return functionName;
}

template <size_t Dim>
void VerticeSet<Dim>::setGLUniforms(QOpenGLShaderProgram& program) const
{
	ImplicitFunction<Dim>::setGLUniforms(program);




	std::vector<float> verticesFlattenData;
	verticesFlattenData.reserve(vertices.size() * 4);
	for (const auto& vertex : vertices)
	{
		verticesFlattenData.push_back(vertex[0]);
		verticesFlattenData.push_back(vertex[1]);
		verticesFlattenData.push_back(vertex[2]);
		if constexpr (Dim > 3)
			verticesFlattenData.push_back(vertex[3]);
		else
			verticesFlattenData.push_back(0.0f);

	}

	QOpenGLContext* context = QOpenGLContext::currentContext();
	if (!context)
		return;
	QOpenGLExtraFunctions* gl = context->extraFunctions();
	gl->initializeOpenGLFunctions();

	auto& buffers = verticeSetBuffers<Dim>();
	auto previousBuffer = buffers.find(this);
	if (previousBuffer != buffers.end())
	{
		gl->glDeleteBuffers(1, &previousBuffer->second);
		buffers.erase(previousBuffer);
	}

	GLuint ssbo = 0;
	gl->glGenBuffers(1, &ssbo);

	gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

	gl->glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		verticesFlattenData.size() * sizeof(float),
		verticesFlattenData.data(),
		GL_DYNAMIC_DRAW
	);

	// Bind to binding point 0. The shader reads one vec4 per vertex.
	gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	buffers.emplace(this, ssbo);

	program.setUniformValue("u_radius", radius);
	program.setUniformValue("u_bvhVertexCount", static_cast<int>(vertices.size()));

}


template <size_t Dim>
const std::string VerticeSet<Dim>::getFragmentShader() const
{
	GLSLContext ctx;
	const std::string rootFunction = buildGLSL(ctx);
	return buildPolytopeFragmentShader(ctx, rootFunction, PolytopeShaderStyle::Node, Dim);
}



template class VerticeSet<2>;
template class VerticeSet<3>;
template class VerticeSet<4>;
