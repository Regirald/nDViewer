#pragma once
#include "core/Vec.h"
#include "ShaderBuilder.h"
#include <string>
#include <algorithm>
#include <memory>
#include <QOpenGLShaderProgram>



template <size_t Dim>
class ImplicitFunction {
public:
    using Point = Vec<Dim, float>;

    virtual ~ImplicitFunction() = default;
    virtual std::shared_ptr<ImplicitFunction<Dim>> clone() const = 0;

    virtual void initializeUniformNames();
    virtual float evaluateSDF(const Point& p) const = 0;

    virtual const std::string buildSDF() const;
    virtual const std::string buildUniforms() const;
    virtual const std::string getVertexShader() const;
    virtual const std::string getFragmentShader() const;
    virtual void setGLUniforms(QOpenGLShaderProgram& program) const;

    QVector3D m_boundsMin;
    QVector3D m_boundsMax;

	std::string m_dstFunctionName = "sampleSDF";
	std::vector<std::string> m_uniformsNames;
};

template <size_t Dim>
inline void ImplicitFunction<Dim>::initializeUniformNames()
{
    m_uniformsNames.push_back("u_boundsMin");
    m_uniformsNames.push_back("u_boundsMax");
}

template <size_t Dim>
inline const std::string ImplicitFunction<Dim>::buildUniforms() const
{
	std::string uniforms = 
        "uniform vec3 " + m_uniformsNames[0] + ";\n"
		"uniform vec3 " + m_uniformsNames[1] + ";\n";

    return uniforms;
};

template <size_t Dim>
inline const std::string ImplicitFunction<Dim>::buildSDF() const
{
	std::string functionSDF = "float " + m_dstFunctionName + "(vec3 p)\n"
		"{\n"
		"    return 0.f;\n"
		"}\n";

    return functionSDF;
};



template <size_t Dim>
inline void ImplicitFunction<Dim>::setGLUniforms(QOpenGLShaderProgram& m_program) const
{
    m_program.setUniformValue(m_uniformsNames[0].c_str(), m_boundsMin);
    m_program.setUniformValue(m_uniformsNames[1].c_str(), m_boundsMax);
}




template <size_t Dim>
inline const std::string ImplicitFunction<Dim>::getVertexShader() const
{
    return ShaderBuilder::buildVertexShader();
}



template <size_t Dim>
inline const std::string ImplicitFunction<Dim>::getFragmentShader() const
{
    std::string uniforms = buildUniforms();
    std::string dstFunction = buildSDF();
    return ShaderBuilder::buildFragmentShader(uniforms, dstFunction);
}