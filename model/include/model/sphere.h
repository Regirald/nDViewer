#pragma once
#include "ImplicitFunction.h"
#include <QVector3D>


class Sphere : public ImplicitFunction<3> {
public:
	Sphere(Vec<3, float> center = { 0.f, 0.f, 0.f }, float radius = 1.f)
        : m_center(center), m_radius(radius){
        initializeUniformNames();
        m_boundsMin = QVector3D(m_center[0] - m_radius, m_center[1] - m_radius, m_center[2] - m_radius);
        m_boundsMax = QVector3D(m_center[0] + m_radius, m_center[1] + m_radius, m_center[2] + m_radius);
    }

    float evaluateSDF(const Point& p) const override {
        return std::sqrt((p[0] - m_center[0]) * (p[0] - m_center[0]) +
            (p[1] - m_center[1]) * (p[1] - m_center[1]) +
            (p[2] - m_center[2]) * (p[2] - m_center[2])) -
            m_radius;
    }

    Vec<3, float> m_center = { 0.f, 0.f, 0.f };
    float m_radius = 1.f;


    const std::string buildSDF() const override
    {
        return "float " + m_dstFunctionName + "(vec3 p)\n"
            "{\n"
            "    return length(p - " + m_uniformsNames[3] + ") - " + m_uniformsNames[2] + ";\n"
            "}\n";
    }

    const std::string buildUniforms() const override
    {
        return ImplicitFunction<3>::buildUniforms()
            + "uniform float " + m_uniformsNames[2] + ";\n"
            + "uniform vec3 " + m_uniformsNames[3] + ";\n";
    }


    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
		ImplicitFunction<3>::setGLUniforms(program);
        program.setUniformValue(m_uniformsNames[3].c_str(), QVector3D(m_center[0], m_center[1], m_center[2]));
        program.setUniformValue(m_uniformsNames[2].c_str(), m_radius);
    }

private:
    void initializeUniformNames()
    {
        m_uniformsNames.push_back("u_radius");
        m_uniformsNames.push_back("u_center");
    }
};