#pragma once
#include "ImplicitFunction.h"
#include <QVector3D>
#include <QOpenGLShaderProgram>


class Sphere : public ImplicitFunction<3> {
public:
	Sphere(Vec<3, float> center = { 0.f, 0.f, 0.f }, float radius = 1.f)
        : center(center), radius(radius) {}

    float evaluateSDF(const Point& p) const override {
        return std::sqrt((p[0] - center[0]) * (p[0] - center[0]) +
            (p[1] - center[1]) * (p[1] - center[1]) +
            (p[2] - center[2]) * (p[2] - center[2])) -
            radius;
    }

    Vec<3, float> center = { 0.f, 0.f, 0.f };
    float radius = 1.f;
    mutable std::string u_radius;
    mutable std::string u_center;


    std::string buildGLSL(GLSLContext& ctx) const override
    {

            std::string fname = ctx.makeName("sdf_sphere");
            u_radius = ctx.makeUniform("u_radius", "float");
            u_center = ctx.makeUniform("u_center", "vec3");

            ctx.append("float " + fname + "(vec3 p) {");
            ctx.append(" vec3 uvw = worldToTex(p); if (any(lessThan(uvw, vec3(0.0))) || any(greaterThan(uvw, vec3(1.0)))) return 1e6;"); // TODO check if usefull
            ctx.append("    return length(p - " + u_center + ") - " + u_radius + ";");
            ctx.append("}");

            return fname;
       }

    const std::string getGLSL_parameters() const override
    {
        return std::string("uniform vec3 center;\nuniform float radius;\n");
    }

    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        program.setUniformValue(u_center.c_str(), QVector3D(center[0], center[1], center[2]));
        program.setUniformValue(u_radius.c_str(), radius);
    }
};