#pragma once
#include "core/Vec.h"
#include "model/ImplicitFunction.h"

#include <string>
#include <vector>
#include <memory>
#include <QOpenGLShaderProgram>



template <size_t Dim>
class ImplicitFunctionSet {
public:
    using Point = Vec<Dim, float>;

    ImplicitFunctionSet(std::unique_ptr<ImplicitFunction<Dim>> refPrimitive) : m_refPrimitive(std::move(refPrimitive)) {}
    virtual ~ImplicitFunctionSet() = default;

    virtual float evaluateSDF(const Point& p) const = 0;


    virtual const std::string getFragmentShader() const;
    virtual const std::string getVertexShader() const;
    virtual void setGLUniforms(QOpenGLShaderProgram& program, int indexCurrentPrimitive) const;



    std::unique_ptr<ImplicitFunction<Dim>> m_refPrimitive;
	int m_primitiveCount = 0;
};



template<size_t Dim>
inline void ImplicitFunctionSet<Dim>::setGLUniforms(QOpenGLShaderProgram& program, int indexCurrentPrimitive) const
{
	m_refPrimitive->setGLUniforms(program);
}

template <size_t Dim>
inline const std::string ImplicitFunctionSet<Dim>::getVertexShader() const
{
    return m_refPrimitive->getVertexShader();
}



template <size_t Dim>
inline const std::string ImplicitFunctionSet<Dim>::getFragmentShader() const
{
    return m_refPrimitive->getFragmentShader();
}