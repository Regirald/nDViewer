#pragma once
#include "core/Vec.h"
#include "model/ImplicitFunction.h"

#include <string>
#include <QOpenGLShaderProgram>



template <size_t Dim>
class ImplicitFunctionSet {
public:
    using Point = Vec<Dim, float>;

    virtual ~ImplicitFunctionSet() = default;

    virtual float evaluateSDF(const Point& p) const = 0;

    virtual const std::string buildSDF() const;
    virtual const std::string buildUniforms() const;
    virtual const std::string getFragmentShader() const;
    virtual const std::string getVertexShader() const;
    virtual void setGLUniforms(QOpenGLShaderProgram& program) const;



    std::vector<ImplicitFunction> m_implicitFunctions;
};

