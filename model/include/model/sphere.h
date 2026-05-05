#pragma once
#include "ImplicitFunction.h"
#include <QVector3D>
#include <QOpenGLShaderProgram>


class Sphere : public ImplicitFunction<3> {
public:
    float evaluateSDF(const Point& p) const override {
        return std::sqrt((p[0] - center[0]) * (p[0] - center[0]) +
            (p[1] - center[1]) * (p[1] - center[1]) +
            (p[2] - center[2]) * (p[2] - center[2])) -
            radius;
    }

    Vec<3, float> center = { 0.f, 0.f, 0.f };
    float radius = 1.f;
    const std::string getGLSL_SDF() const override
    {
        const char* sdf = R"(

        float sampleSDF(vec3 p)
            {
                vec3 uvw = worldToTex(p);

                if (any(lessThan(uvw, vec3(0.0))) || any(greaterThan(uvw, vec3(1.0))))
                    return 1e6;

                return length(p - center) - radius;
            }
        )";
        return std::string(sdf);
    };
    const std::string getGLSL_parameters() const override
    {
        return std::string("uniform vec3 center;\nuniform float radius;\n");
    }

    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        program.setUniformValue("center", QVector3D(center[0], center[1], center[2]));
        program.setUniformValue("radius", radius);
    }
};