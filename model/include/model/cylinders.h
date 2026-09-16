#pragma once
#include "ImplicitFunction.h"
#include <vector>
#include <string>
#include <QVector3D>

class CylindersCollection : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	// Each cylinder is defined by two endpoints and a radius
	CylindersCollection(const std::vector<std::pair<Point, Point>>& segments, const std::vector<float>& radii)
		: segments(segments), radii(radii)
	{}

	float evaluateSDF(const Point& p) const override
	{
		float d = std::numeric_limits<float>::infinity();
		for (size_t i = 0; i < segments.size(); ++i)
		{
			const Point& a = segments[i].first;
			const Point& b = segments[i].second;
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
			d = std::min(d, dist - radii[i]);
		}
		return d;
	}

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_cylinders");

		mutable_u_p0.clear();
		mutable_u_p1.clear();
		mutable_u_r.clear();

		for (size_t i = 0; i < segments.size(); ++i)
		{
			mutable_u_p0.push_back(ctx.makeUniform("u_p0", "vec3"));
			mutable_u_p1.push_back(ctx.makeUniform("u_p1", "vec3"));
			mutable_u_r.push_back(ctx.makeUniform("u_radius", "float"));
		}

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    float d = 1e9;");

		for (size_t i = 0; i < segments.size(); ++i)
		{
			// manually inline distance to segment math in GLSL
			ctx.append("    {");
			ctx.append("        vec3 a = " + mutable_u_p0[i] + ";");
			ctx.append("        vec3 b = " + mutable_u_p1[i] + ";");
			ctx.append("        vec3 ab = b - a;");
			ctx.append("        vec3 ap = p - a;");
			ctx.append("        float c1 = dot(ab, ap);");
			ctx.append("        float c2 = dot(ab, ab);");
			ctx.append("        float t = 0.0;");
			ctx.append("        if (c2 > 1e-8) t = clamp(c1 / c2, 0.0, 1.0);");
			ctx.append("        vec3 proj = a + t * ab;");
			ctx.append("        float dist = length(p - proj);");
			ctx.append("        d = min(d, dist - " + mutable_u_r[i] + ");");
			ctx.append("    }");
		}

		ctx.append("    return d;");
		ctx.append("}");

		return fname;
	}

	const std::string getGLSL_parameters() const override { return std::string(); }

	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);
		for (size_t i = 0; i < segments.size(); ++i)
		{
			program.setUniformValue(mutable_u_p0[i].c_str(), QVector3D(segments[i].first[0], segments[i].first[1], segments[i].first[2]));
			program.setUniformValue(mutable_u_p1[i].c_str(), QVector3D(segments[i].second[0], segments[i].second[1], segments[i].second[2]));
			program.setUniformValue(mutable_u_r[i].c_str(), radii[i]);
		}
	}

	std::vector<std::pair<Point, Point>> segments;
	std::vector<float> radii;

	mutable std::vector<std::string> mutable_u_p0;
	mutable std::vector<std::string> mutable_u_p1;
	mutable std::vector<std::string> mutable_u_r;
};
