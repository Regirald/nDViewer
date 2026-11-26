#pragma once
#include "ImplicitFunction.h"
#include "PolytopeShaderBuilder.h"
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <QVector3D>

class PolytopeNode : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	// Each node is defined by a position and a m_radius
	PolytopeNode(const Point& node, float m_radius)
		: node(node), m_radius(m_radius)
	{
		m_uniformsNames.push_back("u_node");
		m_uniformsNames.push_back("u_radius");
		// initialize m_Bounds
		m_boundsMin[0] = std::min(m_boundsMin[0], node[0] - m_radius);
		m_boundsMin[1] = std::min(m_boundsMin[1], node[1] - m_radius);
		m_boundsMin[2] = std::min(m_boundsMin[2], node[2] - m_radius);
		m_boundsMax[0] = std::max(m_boundsMax[0], node[0] + m_radius);
		m_boundsMax[1] = std::max(m_boundsMax[1], node[1] + m_radius);
		m_boundsMax[2] = std::max(m_boundsMax[2], node[2] + m_radius);
	}


	float evaluateSDF(const Point& p) const override
	{
		float dx = p[0] - node[0];
		float dy = p[1] - node[1];
		float dz = p[2] - node[2];
		float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
		return dist - m_radius;
	}

	const std::string buildSDF() const override
	{
		return "float " + m_dstFunctionName + "(vec3 p) {\n"
			"    vec3 delta = p - " + m_uniformsNames[2] + ";\n"
			"    return length(delta) - " + m_uniformsNames[3] + ";\n"
			"}\n";
	}

	const std::string buildUniforms() const override
	{
		return ImplicitFunction<3>::buildUniforms()
			+ "uniform vec3 " + m_uniformsNames[2] + ";\n"
			+ "uniform float " + m_uniformsNames[3] + ";\n";
	}

	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);
		program.setUniformValue(m_uniformsNames[2].c_str(), QVector3D(node[0], node[1], node[2]));
		program.setUniformValue(m_uniformsNames[3].c_str(), m_radius);
	}


	const std::string getFragmentShader() const override
	{
		return PolytopeShaderBuilder::buildFragmentShader(buildUniforms(), buildSDF(), PolytopeShaderBuilder::PolytopeShaderStyle::Node);
	}

	Point node;
	float m_radius;

};
