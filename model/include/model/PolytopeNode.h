#pragma once
#include "ImplicitFunction.h"
#include "PolytopeShader.h"
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <QVector3D>

class PolytopeNode : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	// Each node is defined by a position and a radius
	PolytopeNode(const Point& node, float radius)
		: node(node), radius(radius)
	{
		// initialize m_Bounds
		m_boundsMin[0] = std::min(m_boundsMin[0], node[0] - radius);
		m_boundsMin[1] = std::min(m_boundsMin[1], node[1] - radius);
		m_boundsMin[2] = std::min(m_boundsMin[2], node[2] - radius);
		m_boundsMax[0] = std::max(m_boundsMax[0], node[0] + radius);
		m_boundsMax[1] = std::max(m_boundsMax[1], node[1] + radius);
		m_boundsMax[2] = std::max(m_boundsMax[2], node[2] + radius);
	}


	float evaluateSDF(const Point& p) const override
	{
		float dx = p[0] - node[0];
		float dy = p[1] - node[1];
		float dz = p[2] - node[2];
		float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
		return dist - radius;
	}

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_node");

		mutable_u_p = ctx.makeUniformUnique("u_p", "vec3");
		mutable_u_r = ctx.makeUniformUnique("u_radius", "float");

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    vec3 delta = p - " + mutable_u_p + ";");
		ctx.append("    float r = " + mutable_u_r + ";");
		ctx.append("    return length(delta) - r;");
		ctx.append("}");

		return fname;
	}

	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);
		program.setUniformValue(mutable_u_p.c_str(), QVector3D(node[0], node[1], node[2]));
		program.setUniformValue(mutable_u_r.c_str(), radius);
	}


	const std::string getFragmentShader() const override
	{
		GLSLContext ctx;
		std::string rootFunc = buildGLSL(ctx);
		return buildPolytopeFragmentShader(ctx, rootFunc, PolytopeShaderStyle::Node);
	}

	Point node;
	float radius;

	mutable std::string mutable_u_p;
	mutable std::string mutable_u_r;
};
