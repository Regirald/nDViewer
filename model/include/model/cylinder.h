#pragma once

#include "ImplicitFunction.h"
#include <QVector3D>


class Cylinder : public ImplicitFunction<3> {
public:
	std::shared_ptr<ImplicitFunction<3>> clone() const override
	{
		return std::make_shared<Cylinder>(*this);
	}



	Cylinder(Vec<3, float> a, Vec<3, float> b, float radius)
		: m_a(a), m_b(b), m_radius(radius) {
		initializeUniformNames();
		m_boundsMin = QVector3D(std::min(m_a[0], m_b[0]) - m_radius, std::min(m_a[1], m_b[1]) - m_radius, std::min(m_a[2], m_b[2]) - m_radius);
		m_boundsMax = QVector3D(std::max(m_a[0], m_b[0]) + m_radius, std::max(m_a[1], m_b[1]) + m_radius, std::max(m_a[2], m_b[2]) + m_radius);
	}

	float evaluateSDF(const Point& p) const override {
		// distance from point p to segment ab
		Vec<3, float> ab = m_b - m_a;
		Vec<3, float> ap = p - m_a;
		float t = std::max(0.0f, std::min(1.0f, (ab[0] * ap[0] + ab[1] * ap[1] + ab[2] * ap[2]) / (ab[0] * ab[0] + ab[1] * ab[1] + ab[2] * ab[2])));
		Vec<3, float> closestPoint = m_a + t * ab;
		float dist = std::sqrt((p[0] - closestPoint[0]) * (p[0] - closestPoint[0]) +
			(p[1] - closestPoint[1]) * (p[1] - closestPoint[1]) +
			(p[2] - closestPoint[2]) * (p[2] - closestPoint[2]));
		return dist - m_radius;
	}

	const std::string buildSDF() const override {
		return "float " + m_dstFunctionName + "(vec3 p) {\n"
			"    vec3 a = " + m_uniformsNames[0] + ";\n"
			"    vec3 b = " + m_uniformsNames[1] + ";\n"
			"    float r = " + m_uniformsNames[2] + ";\n"
			"    vec3 ab = b - a;\n"
			"    float c2 = dot(ab, ab);\n"
			"    float t = c2 > 1e-8 ? clamp(dot(p - a, ab) / c2, 0.0, 1.0) : 0.0;\n"
			"    return length(p - (a + t * ab)) - r;\n"
			"}\n";
	}

	const std::string buildUniforms() const override {
		return ImplicitFunction<3>::buildUniforms()
			+ "uniform vec3 " + m_uniformsNames[0] + ";\n"
			+ "uniform vec3 " + m_uniformsNames[1] + ";\n"
			+ "uniform float " + m_uniformsNames[2] + ";\n";
	}	

	void setGLUniforms(QOpenGLShaderProgram& program) const override {
		ImplicitFunction<3>::setGLUniforms(program);
		program.setUniformValue(m_uniformsNames[0].c_str(), QVector3D(m_a[0], m_a[1], m_a[2]));
		program.setUniformValue(m_uniformsNames[1].c_str(), QVector3D(m_b[0], m_b[1], m_b[2]));
		program.setUniformValue(m_uniformsNames[2].c_str(), m_radius);
	}

private:
	void initializeUniformNames() {
		ImplicitFunction<3>::initializeUniformNames();
		m_uniformsNames.push_back("u_a");
		m_uniformsNames.push_back("u_b");
		m_uniformsNames.push_back("u_radius");
	}
	Vec<3, float> m_a;
	Vec<3, float> m_b;
	float m_radius;
};