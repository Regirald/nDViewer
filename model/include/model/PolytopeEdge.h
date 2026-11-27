#pragma once
#include "ImplicitFunction.h"
#include "PolytopeShaderBuilder.h"
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <QVector3D>
class PolytopeEdge : public ImplicitFunction<3> {
public:
	std::shared_ptr<ImplicitFunction<3>> clone() const override
	{
		return std::make_shared<PolytopeEdge>(*this);
	}

	using Point = ImplicitFunction<3>::Point;

	// A single cylinder defined by two endpoints and a m_radius
	PolytopeEdge(const std::pair<Point, Point>& segment, float r)
		: a(segment.first), b(segment.second), radius(r)
	{
		m_uniformsNames.push_back("u_p0");
		m_uniformsNames.push_back("u_p1");
		m_uniformsNames.push_back("u_radius");
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

	const std::string buildSDF() const override
	{
		return "float " + m_dstFunctionName + "(vec3 p) {\n"
			"    vec3 a = " + m_uniformsNames[2] + ";\n"
			"    vec3 b = " + m_uniformsNames[3] + ";\n"
			"    vec3 ab = b - a;\n"
			"    float c2 = dot(ab, ab);\n"
			"    float t = c2 > 1e-8 ? clamp(dot(p - a, ab) / c2, 0.0, 1.0) : 0.0;\n"
			"    return length(p - (a + t * ab)) - " + m_uniformsNames[4] + ";\n"
			"}\n";
	}

	const std::string buildUniforms() const override
	{
		return ImplicitFunction<3>::buildUniforms()
			+ "uniform vec3 " + m_uniformsNames[2] + ";\n"
			+ "uniform vec3 " + m_uniformsNames[3] + ";\n"
			+ "uniform float " + m_uniformsNames[4] + ";\n";
	}


	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);
		program.setUniformValue(m_uniformsNames[2].c_str(), QVector3D(a[0], a[1], a[2]));
		program.setUniformValue(m_uniformsNames[3].c_str(), QVector3D(b[0], b[1], b[2]));
		program.setUniformValue(m_uniformsNames[4].c_str(), radius);
	}


	const std::string getFragmentShader() const override
	{
		return PolytopeShaderBuilder::buildFragmentShader();
	}

	Point a;
	Point b;
	float radius;

};
