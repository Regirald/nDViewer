#pragma once
#include "ImplicitFunction.h"
#include <vector>
#include <string>
#include <QVector3D>

class PlanesCollection : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	// Each plane defined by a point on the plane and a normal
	PlanesCollection(const std::vector<std::pair<Point, Point>>& planes)
		: planes(planes)
	{}

	float evaluateSDF(const Point& p) const override
	{
		float d = std::numeric_limits<float>::infinity();
		for (const auto& pl : planes)
		{
			const Point& pt = pl.first;
			const Point& n = pl.second;
			float val = (p[0]-pt[0])*n[0] + (p[1]-pt[1])*n[1] + (p[2]-pt[2])*n[2];
			d = std::min(d, std::abs(val));
		}
		return d;
	}

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_planes");

		mutable_u_pt.clear();
		mutable_u_n.clear();
		for (size_t i = 0; i < planes.size(); ++i)
		{
			mutable_u_pt.push_back(ctx.makeUniform("u_plane_pt", "vec3"));
			mutable_u_n.push_back(ctx.makeUniform("u_plane_n", "vec3"));
		}

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    float d = 1e9;");
		for (size_t i = 0; i < planes.size(); ++i)
		{
			ctx.append("    d = min(d, abs(dot(p - " + mutable_u_pt[i] + ", normalize(" + mutable_u_n[i] + "))));");
		}
		ctx.append("    return d;");
		ctx.append("}");

		return fname;
	}

	const std::string getGLSL_parameters() const override { return std::string(); }

	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);
		for (size_t i = 0; i < planes.size(); ++i)
		{
			program.setUniformValue(mutable_u_pt[i].c_str(), QVector3D(planes[i].first[0], planes[i].first[1], planes[i].first[2]));
			program.setUniformValue(mutable_u_n[i].c_str(), QVector3D(planes[i].second[0], planes[i].second[1], planes[i].second[2]));
		}
	}

	std::vector<std::pair<Point, Point>> planes;
	mutable std::vector<std::string> mutable_u_pt;
	mutable std::vector<std::string> mutable_u_n;
};
