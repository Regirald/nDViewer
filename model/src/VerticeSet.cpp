#include "VerticeSet.h"

template <size_t Dim>
VerticeSet<Dim>::VerticeSet(
	std::vector<Point> vertices,
	std::vector<Node> nodes,
	std::vector<Primitive> primitives,
	float radius)
	: vertices(vertices),
	  nodes(nodes),
	  primitives(primitives),
	  radius(radius)
{
	if (!this->nodes.empty())
	{
		const Node& root = this->nodes.front();

		QVector3D rootBoundsMin;
		QVector3D rootBoundsMax;
		for (int i = 0; i < std::min((int)Dim, 3); ++i)
		{
			rootBoundsMin[i] = root.bounds.min[i];
			rootBoundsMax[i] = root.bounds.max[i];
		}

		ImplicitFunction<Dim>::m_boundsMin = rootBoundsMin * 4;
		ImplicitFunction<Dim>::m_boundsMax = rootBoundsMax * 4;
	}

	packBVHData();
}

template <size_t Dim>
VerticeSet<Dim>::~VerticeSet()
{
	QOpenGLContext* context = QOpenGLContext::currentContext();
	if (!context)
		return;

	QOpenGLExtraFunctions* gl = context->extraFunctions();
	gl->initializeOpenGLFunctions();
	if (m_bvhTexture)
		gl->glDeleteTextures(1, &m_bvhTexture);
	if (m_bvhBuffer)
		gl->glDeleteBuffers(1, &m_bvhBuffer);
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

	ctx.uniforms.append("uniform samplerBuffer u_bvhData;\n");
	ctx.uniforms.append("uniform int u_bvhVertexCount;\n");


	// BVH bounds distance.

	// Main BVH traversal.

	ctx.append("float " + functionName + "(vec3 p) {");

	ctx.append("	float bestDistance = 1e10;");
	ctx.append("	for (int vertexIndex = 0; vertexIndex < u_bvhVertexCount; ++vertexIndex)");
	ctx.append("	{");

	ctx.append("		int vertexOffset = vertexIndex * 5;");
	ctx.append("		vec4 vertex = vec4(		texelFetch(u_bvhData, vertexOffset + 0).r, texelFetch(u_bvhData, vertexOffset + 1).r,texelFetch(u_bvhData, vertexOffset + 2).r,	texelFetch(u_bvhData, vertexOffset + 3).r	);");
	ctx.append("		vec3 vertex3d = vertex.xyz / (1 - 0.3 * vertex.w);");
	ctx.append("		float vertexRadius = texelFetch(u_bvhData, vertexOffset + 4).r;");
	ctx.append("		float newDistance = length(p - vertex3d) - vertexRadius;");
	ctx.append("		if(newDistance < bestDistance)");
	ctx.append("		{");
	ctx.append("			u_primitiveToutchedIndex = vertexIndex;");
	ctx.append("			bestDistance = newDistance;");
	ctx.append("		}");
	ctx.append("	}");

	ctx.append("    return bestDistance;");
	ctx.append("}");


	ctx.append("float SDFonPrimitive(vec3 p) {");

	ctx.append("	float bestDistance = 1e10;");
	ctx.append("	int vertexIndex = u_primitiveToutchedIndex;");

	ctx.append("	int vertexOffset = vertexIndex * 5;");
	ctx.append("	vec4 vertex = vec4(texelFetch(u_bvhData, vertexOffset + 0).r, texelFetch(u_bvhData, vertexOffset + 1).r,texelFetch(u_bvhData, vertexOffset + 2).r,	texelFetch(u_bvhData, vertexOffset + 3).r	);");
	ctx.append("	vec3 vertex3d = vertex.xyz / (1 - 0.3 * vertex.w);");
	ctx.append("	float vertexRadius = texelFetch(u_bvhData, vertexOffset + 4).r;");
	ctx.append("	bestDistance = min(bestDistance, length(p - vertex3d) - vertexRadius);");

	ctx.append("    return bestDistance;");
	ctx.append("}");

	return functionName;
}

template <size_t Dim>
void VerticeSet<Dim>::setGLUniforms(QOpenGLShaderProgram& program) const
{
	ImplicitFunction<Dim>::setGLUniforms(program);
	ensureBVHBuffer();

	constexpr int textureUnit = 5;
	if (QOpenGLContext::currentContext())
	{
		QOpenGLExtraFunctions* gl = QOpenGLContext::currentContext()->extraFunctions();
		gl->glActiveTexture(GL_TEXTURE0 + textureUnit);
		gl->glBindTexture(GL_TEXTURE_BUFFER, m_bvhTexture);
		gl->glActiveTexture(GL_TEXTURE0);
	}
	program.setUniformValue("u_bvhData", textureUnit);
	program.setUniformValue("u_bvhVertexCount", static_cast<int>(vertices.size()));
}

template <size_t Dim>
const std::string VerticeSet<Dim>::getFragmentShader() const
{
	GLSLContext ctx;
	const std::string rootFunction = buildGLSL(ctx);
	return buildPolytopeFragmentShader(ctx, rootFunction, PolytopeShaderStyle::Node, Dim);
}

template <size_t Dim>
void VerticeSet<Dim>::packBVHData()
{
	m_packedData.clear();

	m_packedData.reserve(vertices.size() * (Dim + 1) 
	);

	const auto appendTexel = [this](Point p)
		{
			for (size_t i = 0; i < Dim; ++i)
				m_packedData.push_back(p[i]);
		};



	for (const Point& vertex : vertices)
	{
		appendTexel(vertex);
		m_packedData.push_back(radius);
	}


}

template <size_t Dim>
void VerticeSet<Dim>::ensureBVHBuffer() const
{
	if (m_bvhBuffer || m_packedData.empty())
		return;

	QOpenGLContext* context = QOpenGLContext::currentContext();
	if (!context)
		return;

	QOpenGLExtraFunctions* gl = context->extraFunctions();
	gl->initializeOpenGLFunctions();
	gl->glGenBuffers(1, &m_bvhBuffer);
	gl->glBindBuffer(GL_TEXTURE_BUFFER, m_bvhBuffer);
	gl->glBufferData(
		GL_TEXTURE_BUFFER,
		static_cast<GLsizeiptr>(m_packedData.size() * sizeof(float)),
		m_packedData.data(),GL_STATIC_DRAW);
	gl->glGenTextures(1, &m_bvhTexture);
	gl->glBindTexture(GL_TEXTURE_BUFFER, m_bvhTexture);
	gl->glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_bvhBuffer);
	gl->glBindTexture(GL_TEXTURE_BUFFER, 0);
	gl->glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

template class VerticeSet<2>;
template class VerticeSet<3>;
template class VerticeSet<4>;
