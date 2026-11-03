#pragma once
#include "ImplicitFunction.h"
#include "PolytopeShader.h"
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <QVector3D>
class PolytopeEdge : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	// A single cylinder defined by two endpoints and a radius
	PolytopeEdge(const std::pair<Point, Point>& segment, float r)
		: a(segment.first), b(segment.second), radius(r)
	{
		// initialize bounds
		m_boundsMin[0] = std::min(m_boundsMin[0], std::min(a[0], b[0]) - radius);
		m_boundsMin[1] = std::min(m_boundsMin[1], std::min(a[1], b[1]) - radius);
		m_boundsMin[2] = std::min(m_boundsMin[2], std::min(a[2], b[2]) - radius);
		m_boundsMax[0] = std::max(m_boundsMax[0], std::max(a[0], b[0]) + radius);
		m_boundsMax[1] = std::max(m_boundsMax[1], std::max(a[1], b[1]) + radius);
		m_boundsMax[2] = std::max(m_boundsMax[2], std::max(a[2], b[2]) + radius);
	}

	float evaluateSDF(const Point& p) const override
	{
		// distance from point p to segment ab
		float vx = b[0] - a[0];
		float vy = b[1] - a[1];
		float vz = b[2] - a[2];
		float wx = p[0] - a[0];
		float wy = p[1] - a[1];
		float wz = p[2] - a[2];
		float c1 = vx*wx + vy*wy + vz*wz;
		float c2 = vx*vx + vy*vy + vz*vz;
		float t = 0.0f;
		if (c2 > 1e-8f) t = std::max(0.0f, std::min(1.0f, c1 / c2));
		float projx = a[0] + t * vx;
		float projy = a[1] + t * vy;
		float projz = a[2] + t * vz;
		float dist = std::sqrt((p[0]-projx)*(p[0]-projx) + (p[1]-projy)*(p[1]-projy) + (p[2]-projz)*(p[2]-projz));
		return dist - radius;
	}

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_edge");

		mutable_u_p0 = ctx.makeUniformUnique("u_p0", "vec3");
		mutable_u_p1 = ctx.makeUniformUnique("u_p1", "vec3");
		mutable_u_r = ctx.makeUniformUnique("u_radius", "float");

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    vec3 a = " + mutable_u_p0 + ";");
		ctx.append("    vec3 b = " + mutable_u_p1 + ";");
		ctx.append("    vec3 ab = b - a;");
		ctx.append("    vec3 ap = p - a;");
		ctx.append("    float c1 = dot(ab, ap);");
		ctx.append("    float c2 = dot(ab, ab);");
		ctx.append("    float t = 0.0;");
		ctx.append("    if (c2 > 1e-8) t = clamp(c1 / c2, 0.0, 1.0);");
		ctx.append("    vec3 proj = a + t * ab;");
		ctx.append("    float dist = length(p - proj);");
		ctx.append("    return dist - " + mutable_u_r + ";");
		ctx.append("}");

		return fname;
	}


	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);
		program.setUniformValue(mutable_u_p0.c_str(), QVector3D(a[0], a[1], a[2]));
		program.setUniformValue(mutable_u_p1.c_str(), QVector3D(b[0], b[1], b[2]));
		program.setUniformValue(mutable_u_r.c_str(), radius);
	}


	const std::string getFragmentShader() const override
	{
		GLSLContext ctx;
		std::string rootFunc = buildGLSL(ctx);
		return buildPolytopeFragmentShader(ctx, rootFunc, PolytopeShaderStyle::Edge);
	}

	Point a;
	Point b;
	float radius;

	mutable std::string mutable_u_p0;
	mutable std::string mutable_u_p1;
	mutable std::string mutable_u_r;
};
