#pragma once

#include "ImplicitFunction.h"
#include "PolytopeShader.h"
#include "AccelerationStructureBVH.h"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QVector3D>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

template <size_t Dim>
class VerticeSet : public ImplicitFunction<Dim>
{
public:
	using Point = typename ImplicitFunction<Dim>::Point;
	using Node = typename AccelerationStructureBVH<Dim>::Node;
	using Primitive = typename AccelerationStructureBVH<Dim>::Primitive;



	VerticeSet(
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

	~VerticeSet() override
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

	float evaluateSDF(const Point& p) const override
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

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		const std::string functionName = "sampleVertexBVH";
		ctx.uniforms = "#define POLYTOPE_VERTEX_BVH\n" + ctx.uniforms;

		ctx.append("float bvhBoundsDistance(vec3 p, int nodeIndex) {");
		ctx.append("    vec3 boundsMin = texelFetch(u_bvhData, nodeIndex * 3).xyz;");
		ctx.append("    vec3 boundsMax = texelFetch(u_bvhData, nodeIndex * 3 + 1).xyz;");
		ctx.append("    vec3 outside = max(max(boundsMin - p, p - boundsMax), vec3(0.0));");
		ctx.append("    return length(outside) - texelFetch(u_bvhData, nodeIndex * 3).w;");
		ctx.append("}");
		ctx.append("float " + functionName + "(vec3 p) {");
		ctx.append("    if (u_bvhNodeCount == 0) return 1e20;");
		ctx.append("    int stack[64];");
		ctx.append("    int stackSize = 1;");
		ctx.append("    int visited = 0;");
		ctx.append("    float bestDistance = 1e20;");
		ctx.append("    stack[0] = 0;");
		ctx.append("    while (stackSize > 0 && visited < u_bvhNodeCount) {");
		ctx.append("        int nodeIndex = stack[--stackSize];");
		ctx.append("        ++visited;");
		ctx.append("        if (bvhBoundsDistance(p, nodeIndex) > bestDistance) continue;");
		ctx.append("        vec4 metadata = texelFetch(u_bvhData, nodeIndex * 3 + 2);");
		ctx.append("        int primitiveCount = int(metadata.w);");
		ctx.append("        if (primitiveCount > 0) {");
		ctx.append("            int firstPrimitive = int(metadata.z);");
		ctx.append("            for (int i = 0; i < primitiveCount; ++i) {");
		ctx.append("                int vertexIndex = int(texelFetch(u_bvhData, u_bvhPrimitiveOffset + firstPrimitive + i).x);");
		ctx.append("                vec4 vertex = texelFetch(u_bvhData, u_bvhVertexOffset + vertexIndex);");
		ctx.append("                bestDistance = min(bestDistance, length(p - vertex.xyz) - vertex.w);");
		ctx.append("            }");
		ctx.append("        } else {");
		ctx.append("            int leftChild = int(metadata.x);");
		ctx.append("            int rightChild = int(metadata.y);");
		ctx.append("            if (leftChild >= 0 && bvhBoundsDistance(p, leftChild) <= bestDistance && stackSize < 64) stack[stackSize++] = leftChild;");
		ctx.append("            if (rightChild >= 0 && bvhBoundsDistance(p, rightChild) <= bestDistance && stackSize < 64) stack[stackSize++] = rightChild;");
		ctx.append("        }");
		ctx.append("    }");
		ctx.append("    return bestDistance;");
		ctx.append("}");
		return functionName;
	}

	void setGLUniforms(QOpenGLShaderProgram& program) const override
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
		program.setUniformValue("u_bvhVertexOffset", static_cast<int>(nodes.size() * 3));
		program.setUniformValue("u_bvhPrimitiveOffset", static_cast<int>(nodes.size() * 3 + vertices.size()));
	}

	const std::string getFragmentShader() const override
	{
		GLSLContext ctx;
		const std::string rootFunction = buildGLSL(ctx);
		return buildPolytopeFragmentShader(ctx, rootFunction, PolytopeShaderStyle::Node);
	}

	std::vector<Point> vertices;
	std::vector<Node> nodes;
	std::vector<Primitive> primitives;
	float radius;

private:
	mutable GLuint m_bvhBuffer = 0;
	mutable GLuint m_bvhTexture = 0;
	std::vector<float> m_packedData;

	void packBVHData()
	{
		m_packedData.reserve(nodes.size() * 12 + vertices.size() * 4 + primitives.size() * 4);
		const auto appendTexel = [this](float x, float y, float z, float w)
		{
			m_packedData.insert(m_packedData.end(), { x, y, z, w });
		};

		for (const Node& node : nodes)
		{
			appendTexel(node.bounds.min[0], node.bounds.min[1], node.bounds.min[2], radius);
			appendTexel(node.bounds.max[0], node.bounds.max[1], node.bounds.max[2], 0.0f);
			appendTexel(
				node.primitiveCount == 0 ? static_cast<float>(node.leftChild) : -1.0f,
				node.primitiveCount == 0 ? static_cast<float>(node.rightChild) : -1.0f,
				static_cast<float>(node.firstPrimitive),
				static_cast<float>(node.primitiveCount));
		}

		for (const Point& vertex : vertices)
			appendTexel(vertex[0], vertex[1], vertex[2], radius);

		for (const Primitive& primitive : primitives)
			appendTexel(static_cast<float>(primitive.index), 0.0f, 0.0f, 0.0f);
	}

	void ensureBVHBuffer() const
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
			m_packedData.data(),
			GL_STATIC_DRAW);
		gl->glGenTextures(1, &m_bvhTexture);
		gl->glBindTexture(GL_TEXTURE_BUFFER, m_bvhTexture);
		gl->glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_bvhBuffer);
		gl->glBindTexture(GL_TEXTURE_BUFFER, 0);
		gl->glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}
};


template class VerticeSet<2>;
template class VerticeSet<3>;
template class VerticeSet<4>;

