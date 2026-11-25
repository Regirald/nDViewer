#pragma once
#include "ImplicitFunction.h"
#include <QVector3D>


/// DEPRECIATED FUNCTION

class Sphere : public ImplicitFunction<3> {
public:
	Sphere(Vec<3, float> center = { 0.f, 0.f, 0.f }, float radius = 1.f)
        : center(center), radius(radius){
        initializeUniformNames();
        m_boundsMin = QVector3D(center[0] - radius, center[1] - radius, center[2] - radius);
        m_boundsMax = QVector3D(center[0] + radius, center[1] + radius, center[2] + radius);
    }

    float evaluateSDF(const Point& p) const override {
        return std::sqrt((p[0] - center[0]) * (p[0] - center[0]) +
            (p[1] - center[1]) * (p[1] - center[1]) +
            (p[2] - center[2]) * (p[2] - center[2])) -
            radius;
    }

    Vec<3, float> center = { 0.f, 0.f, 0.f };
    float radius = 1.f;
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
        program.setUniformValue(m_uniformsNames[3].c_str(), QVector3D(center[0], center[1], center[2]));
        program.setUniformValue(m_uniformsNames[2].c_str(), radius);
    }

private:
    void initializeUniformNames()
    {
        m_uniformsNames.push_back("u_radius");
        m_uniformsNames.push_back("u_center");
    }
};