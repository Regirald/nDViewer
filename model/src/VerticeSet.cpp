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
		Point rootBoundsMin = root.bounds.min;
		Point rootBoundsMax = root.bounds.max;

		ImplicitFunction<Dim>::m_boundsMin = QVector3D(rootBoundsMin[0], rootBoundsMin[1], rootBoundsMin[2]);
		ImplicitFunction<Dim>::m_boundsMax = QVector3D(rootBoundsMax[0], rootBoundsMax[1], rootBoundsMax[2]);
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
	const std::string functionName = "sampleVertexBVH";

	ctx.uniforms.append("uniform samplerBuffer u_bvhData;");
	ctx.uniforms.append("uniform int u_bvhNodeCount;");
	ctx.uniforms.append("uniform int u_bvhVertexOffset;");
	ctx.uniforms.append("uniform int u_bvhPrimitiveOffset;");

	// BVH bounds distance.

	ctx.append("float bvhBoundsDistance(" + vectorType + " p, int nodeOffset) {");
	ctx.append("    " + vectorType + " boundsMin;");
	ctx.append("    " + vectorType + " boundsMax;");
	ctx.append("    for (int i = 0; i < " + dim + "; ++i) {");
	ctx.append("        boundsMin[i] = texelFetch(u_bvhData, nodeOffset + i).r;");
	ctx.append("        boundsMax[i] = texelFetch(u_bvhData, nodeOffset + " + dim + " + i).r;");
	ctx.append("    }");
	ctx.append("    " + vectorType + " outside = max(max(boundsMin - p, p - boundsMax), " + vectorType + "(0.0));");
	ctx.append("    float nodeRadius = texelFetch(u_bvhData, nodeOffset + " + std::to_string(Dim * 2 + 4) + ").r;");
	ctx.append("    return length(outside) - nodeRadius;");
	ctx.append("}");

	// Main BVH traversal.

	ctx.append("float " + functionName + "(" + vectorType + " p) {");
	ctx.append("    if (u_bvhNodeCount == 0) return 1e20;");
	ctx.append("    int stack[64];");
	ctx.append("    int stackSize = 1;");
	ctx.append("    int visited = 0;");
	ctx.append("    float bestDistance = 1e20;");
	ctx.append("    stack[0] = 0;");
	ctx.append("    while (stackSize > 0 && visited < u_bvhNodeCount) {");
	ctx.append("        int nodeIndex = stack[--stackSize];");
	ctx.append("        ++visited;");

	// Each node occupies (2 * Dim + 5) floats.

	ctx.append("        int nodeOffset = nodeIndex * " + std::to_string(Dim * 2 + 5) + ";");
	ctx.append("        if (bvhBoundsDistance(p, nodeOffset) > bestDistance) continue;");

	// Node metadata.

	ctx.append("        int metadataOffset = nodeOffset + " + std::to_string(Dim * 2) + ";");
	ctx.append("        int leftChild = int(texelFetch(u_bvhData, metadataOffset).r);");
	ctx.append("        int rightChild = int(texelFetch(u_bvhData, metadataOffset + 1).r);");
	ctx.append("        int firstPrimitive = int(texelFetch(u_bvhData, metadataOffset + 2).r);");
	ctx.append("        int primitiveCount = int(texelFetch(u_bvhData, metadataOffset + 3).r);");
	ctx.append("        if (primitiveCount > 0) {");
	ctx.append("            for (int i = 0; i < primitiveCount; ++i) {");

	// Primitive index.

	ctx.append("                int primitiveOffset = u_bvhPrimitiveOffset + firstPrimitive + i;");
	ctx.append("                int vertexIndex = int(texelFetch(u_bvhData, primitiveOffset).r);");  // here vertexIndex = primitive.index

	// Vertex data.

	ctx.append("                int vertexOffset = u_bvhVertexOffset + vertexIndex * " + std::to_string(Dim + 1) + ";");
	ctx.append("                " + vectorType + " vertex;");
	ctx.append("                for (int i = 0; i < " + dim + "; ++i)");
	ctx.append("                    vertex[i] = texelFetch(u_bvhData, vertexOffset + i).r;");
	ctx.append("                float vertexRadius = texelFetch(u_bvhData, vertexOffset + " + dim + ").r;");
	ctx.append("                bestDistance = min(bestDistance, length(p - vertex) - vertexRadius);");
	ctx.append("            }");
	ctx.append("        } else {");

	// Child nodes.

	ctx.append("            int leftOffset = leftChild * " + std::to_string(Dim * 2 + 5) + ";");
	ctx.append("            int rightOffset = rightChild * " + std::to_string(Dim * 2 + 5) + ";");
	ctx.append("            if (leftChild >= 0 && bvhBoundsDistance(p, leftOffset) <= bestDistance && stackSize < 64)");
	ctx.append("                stack[stackSize++] = leftChild;");
	ctx.append("            if (rightChild >= 0 && bvhBoundsDistance(p, rightOffset) <= bestDistance && stackSize < 64)");
	ctx.append("                stack[stackSize++] = rightChild;");
	ctx.append("        }");
	ctx.append("    }");
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
	program.setUniformValue("u_bvhNodeCount", static_cast<int>(nodes.size()));
	program.setUniformValue("u_bvhVertexOffset", static_cast<int>(nodes.size() * (Dim * 2 + 5)));
	program.setUniformValue("u_bvhPrimitiveOffset", static_cast<int>(nodes.size() * (Dim * 2 + 5) + vertices.size() * (Dim + 1)));
}

template <size_t Dim>
const std::string VerticeSet<Dim>::getFragmentShader() const
{
	GLSLContext ctx;
	const std::string rootFunction = buildGLSL(ctx);
	return buildPolytopeFragmentShader(ctx, rootFunction, PolytopeShaderStyle::Node);
}

template <size_t Dim>
void VerticeSet<Dim>::packBVHData()
{
	m_packedData.clear();

	m_packedData.reserve(
		nodes.size() * (Dim * 2 + 5) +
		vertices.size() * (Dim + 1) +
		primitives.size()
	);

	const auto appendTexel = [this](Point p)
		{
			for (size_t i = 0; i < Dim; ++i)
				m_packedData.push_back(p[i]);
		};

	for (const Node& node : nodes)
	{
		appendTexel(node.bounds.min);
		appendTexel(node.bounds.max);

		m_packedData.push_back(node.primitiveCount == 0	? static_cast<float>(node.leftChild) : -1.0f);
		m_packedData.push_back(node.primitiveCount == 0	? static_cast<float>(node.rightChild) : -1.0f);
		m_packedData.push_back(static_cast<float>(node.firstPrimitive));
		m_packedData.push_back(static_cast<float>(node.primitiveCount));
		m_packedData.push_back(radius);
	}

	for (const Point& vertex : vertices)
	{
		appendTexel(vertex);
		m_packedData.push_back(radius);
	}

	for (const Primitive& primitive : primitives)
		m_packedData.push_back(static_cast<float>(primitive.index));
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
