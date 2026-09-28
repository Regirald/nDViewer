#pragma once
#include "ImplicitFunction.h"
#include <vector>
#include <string>
#include <QVector3D>

class SpheresCollection : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	SpheresCollection(const std::vector<Point>& centers, const std::vector<float>& radii)
		: centers(centers), radii(radii)
	{}

	float evaluateSDF(const Point& p) const override
	{
		float d = std::numeric_limits<float>::infinity();
		for (size_t i = 0; i < centers.size(); ++i)
		{
			float cur = std::sqrt((p[0] - centers[i][0])*(p[0] - centers[i][0]) +
								  (p[1] - centers[i][1])*(p[1] - centers[i][1]) +
								  (p[2] - centers[i][2])*(p[2] - centers[i][2])) - radii[i];
			d = std::min(d, cur);
		}
		return d;
	}

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_spheres");

		// create per-sphere uniforms and record their names
		mutable_u_centers.clear();
		mutable_u_radii.clear();
		for (size_t i = 0; i < centers.size(); ++i)
		{
			mutable_u_centers.push_back(ctx.makeUniformUnique("u_center", "vec3"));
			mutable_u_radii.push_back(ctx.makeUniformUnique("u_radius", "float"));
		}

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    float d = 1e9;");
		for (size_t i = 0; i < centers.size(); ++i)
		{
			ctx.append("    d = min(d, length(p - " + mutable_u_centers[i] + ") - " + mutable_u_radii[i] + ");");
		}
		ctx.append("    return d;");
		ctx.append("}");

		return fname;
	}

	const std::string getGLSL_parameters() const override
	{
		return std::string();
	}

	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);

		// set individual uniforms
		for (size_t i = 0; i < centers.size(); ++i)
		{
			const std::string& nameC = mutable_u_centers[i];
			const std::string& nameR = mutable_u_radii[i];
			program.setUniformValue(nameC.c_str(), QVector3D(centers[i][0], centers[i][1], centers[i][2]));
			program.setUniformValue(nameR.c_str(), radii[i]);
		}
	}

	std::vector<Point> centers;
	std::vector<float> radii;

	// mutable only so setGLUniforms can use them (buildGLSL is const)
	mutable std::vector<std::string> mutable_u_centers;
	mutable std::vector<std::string> mutable_u_radii;
};
