#pragma once
#include "ImplicitFunction.h"


static struct GLSLContext
{
    int counter = 0;

    std::string makeName(const std::string& base)
    {
        return base + "_" + std::to_string(counter++);
    }

    std::string makeUniformUnique(std::string& uniforms, const std::string& base, const std::string& type)
    {
        std::string name = makeName(base);
        uniforms += "uniform " + type + " " + name + ";\n";
        return name;
    }

    std::string makeUniform(std::string& uniforms, const std::string& base, const std::string& type)
    {
        uniforms += "uniform " + type + " " + base + ";\n";
        return base;
    }


};


// helper smooth min implementation used by several operators
static inline float smoothMinFloat(float a, float b, float k)
{
    if (k <= 0.0f)
        return std::min(a, b);

    float h = std::max(0.0f, std::min(1.0f, 0.5f + 0.5f * (b - a) / k));
    float res = b * (1.0f - h) + a * h - k * h * (1.0f - h);
    return res;
}


template <size_t Dim>
class UnionImplicitFunction : public ImplicitFunction<Dim>
{
public:
    using Point = typename ImplicitFunction<Dim>::Point;
    UnionImplicitFunction(std::shared_ptr<ImplicitFunction<Dim>> a,
        std::shared_ptr<ImplicitFunction<Dim>> b,
        bool smooth = false,
        float k = 0.0f)
        : m_a(a), m_b(b), m_smooth(smooth), m_k(k) 
    {
        this->m_boundsMin = QVector3D(
            std::min(a->m_boundsMin.x(), b->m_boundsMin.x()),
            std::min(a->m_boundsMin.y(), b->m_boundsMin.y()),
            std::min(a->m_boundsMin.z(), b->m_boundsMin.z()));
        this->m_boundsMax = QVector3D(
            std::max(a->m_boundsMax.x(), b->m_boundsMax.x()),
            std::max(a->m_boundsMax.y(), b->m_boundsMax.y()),
            std::max(a->m_boundsMax.z(), b->m_boundsMax.z()));

        m_a->m_dstFunctionName = GLSLContext::makeName(m_a->m_dstFunctionName);
        m_b->m_dstFunctionName = GLSLContext::makeName(m_b->m_dstFunctionName);
        this->m_dstFunctionName = GLSLContext::makeName("sdf_union");

        this->m_uniformsNames.push_back(GLSLContext::makeName("u_k"));

        for (auto& name : m_a->m_uniformsNames)
            name = GLSLContext::makeName(name);

        for (auto& name : m_b->m_uniformsNames)
            name = GLSLContext::makeName(name);

        for (auto& name : this->m_uniformsNames)
            name = GLSLContext::makeName(name);
    }

    float evaluateSDF(const Point& p) const override
    {
        float d1 = m_a->evaluateSDF(p);
        float d2 = m_b->evaluateSDF(p);
        if (m_smooth && m_k > 0.0f)
            return smoothMinFloat(d1, d2, m_k);
        return std::min(d1, d2);
    }

    inline const std::string buildUniforms() const
    {
        std::string uA = m_a->buildUniforms();
        std::string uB = m_b->buildUniforms();

		std::string uniforms0 = ImplicitFunction<Dim>::buildUniforms();
        std::string uniforms1 = "uniform float " + this->m_uniformsNames.back() + ";\n";

        return uA + uB + uniforms0 + uniforms1;
    };

    std::string buildSDF() const override
    {

        std::string fA = m_a->buildSDF();
        std::string fB = m_b->buildSDF();

        std::string unionSDF = "float " + this->m_dstFunctionName + "(vec3 p)\n"
			"{\n"
			"    float d1 = " + m_a->m_dstFunctionName + "(p);\n"
			"    float d2 = " + m_b->m_dstFunctionName + "(p);\n";

        if (m_smooth)
        {
            unionSDF.append("    float k = float " + this->m_uniformsNames.back() + ";\n");
            unionSDF.append("    if (k <= 0.0) return min(d1, d2);\n");
            unionSDF.append("    float h = clamp(0.5 + 0.5*(d2 - d1)/k, 0.0, 1.0);\n");
            unionSDF.append("    return mix(d2, d1, h) - k*h*(1.0 - h);\n");
        }
        else
        {
            unionSDF.append("    return min(d1, d2);\n");
        }
        unionSDF.append("}\n");

        return fA + fB + unionSDF;
    }
    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        m_a->setGLUniforms(program);
        m_b->setGLUniforms(program);
        if (m_smooth)
            program.setUniformValue(this->m_uniformsNames.back(), m_k);
        ImplicitFunction<3>::setGLUniforms(program); //overwrite the boundaries

    }

private:
    std::shared_ptr<ImplicitFunction<Dim>> m_a, m_b;
    bool m_smooth = false;
    float m_k = 0.0f;
};

template <size_t Dim>
inline std::shared_ptr<ImplicitFunction<Dim>>
operator+(std::shared_ptr<ImplicitFunction<Dim>> lhs,
    std::shared_ptr<ImplicitFunction<Dim>> rhs)
{
    return std::make_shared<UnionImplicitFunction<Dim>>(lhs, rhs, true, 0.5f);
}


// Intersection: max(d1, d2)
template <size_t Dim>
class IntersectionImplicitFunction : public ImplicitFunction<Dim>
{
public:
    using Point = typename ImplicitFunction<Dim>::Point;

    IntersectionImplicitFunction(std::shared_ptr<ImplicitFunction<Dim>> a,
        std::shared_ptr<ImplicitFunction<Dim>> b,
        bool smooth = false,
        float k = 0.0f)
        : m_a(a), m_b(b), m_smooth(smooth), m_k(k)
    {
        this->m_boundsMin = QVector3D(
            std::min(a->m_boundsMin.x(), b->m_boundsMin.x()),
            std::min(a->m_boundsMin.y(), b->m_boundsMin.y()),
            std::min(a->m_boundsMin.z(), b->m_boundsMin.z()));
        this->m_boundsMax = QVector3D(
            std::max(a->m_boundsMax.x(), b->m_boundsMax.x()),
            std::max(a->m_boundsMax.y(), b->m_boundsMax.y()),
            std::max(a->m_boundsMax.z(), b->m_boundsMax.z()));

        m_a->m_dstFunctionName = GLSLContext::makeName(m_a->m_dstFunctionName);
        m_b->m_dstFunctionName = GLSLContext::makeName(m_b->m_dstFunctionName);
        this->m_dstFunctionName = GLSLContext::makeName("sdf_intersection");

        this->m_uniformsNames.push_back(GLSLContext::makeName("u_k"));

        for (auto& name : m_a->m_uniformsNames)
            name = GLSLContext::makeName(name);

        for (auto& name : m_b->m_uniformsNames)
            name = GLSLContext::makeName(name);

        for (auto& name : this->m_uniformsNames)
            name = GLSLContext::makeName(name);
    }

    float evaluateSDF(const Point& p) const override
    {
        float d1 = m_a->evaluateSDF(p);
        float d2 = m_b->evaluateSDF(p);
        if (m_smooth && m_k > 0.0f)
            return -smoothMinFloat(-d1, -d2, m_k); // smooth max
        return std::max(d1, d2);
    }

    inline const std::string buildUniforms() const override
    {
        std::string uA = m_a->buildUniforms();
        std::string uB = m_b->buildUniforms();

        std::string uniforms0 = ImplicitFunction<Dim>::buildUniforms();
        std::string uniforms1 = "uniform float " + this->m_uniformsNames.back() + ";\n";

        return uA + uB + uniforms0 + uniforms1;
    }

    std::string buildSDF() const override
    {
        std::string fA = m_a->buildSDF();
        std::string fB = m_b->buildSDF();

        std::string intersectionSDF = "float " + this->m_dstFunctionName + "(vec3 p)\n"
            "{\n"
            "    float d1 = " + m_a->m_dstFunctionName + "(p);\n"
            "    float d2 = " + m_b->m_dstFunctionName + "(p);\n";
        if (m_smooth)
        {
            intersectionSDF.append("    float k = " + this->m_uniformsNames.back() + ";\n");
            intersectionSDF.append("    if (k <= 0.0) return max(d1, d2);\n");
            intersectionSDF.append("    float nd1 = -d1; float nd2 = -d2;\n");
            intersectionSDF.append("    float h = clamp(0.5 + 0.5*(nd2 - nd1)/k, 0.0, 1.0);\n");
            intersectionSDF.append("    float s = mix(nd2, nd1, h) - k*h*(1.0 - h);\n");
            intersectionSDF.append("    return -s;\n");
        }
        else
        {
            intersectionSDF.append("    return max(d1, d2);\n");
        }
        intersectionSDF.append("}\n");

        return fA + fB + intersectionSDF;
    }

    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        m_a->setGLUniforms(program);
        m_b->setGLUniforms(program);
        if (m_smooth)
            program.setUniformValue(this->m_uniformsNames.back().c_str(), m_k);
        ImplicitFunction<3>::setGLUniforms(program);
    }

private:
    std::shared_ptr<ImplicitFunction<Dim>> m_a, m_b;
    bool m_smooth = false;
    float m_k = 0.0f;
};

template <size_t Dim>
inline std::shared_ptr<ImplicitFunction<Dim>>
operator*(std::shared_ptr<ImplicitFunction<Dim>> lhs,
    std::shared_ptr<ImplicitFunction<Dim>> rhs)
{
    return std::make_shared<IntersectionImplicitFunction<Dim>>(lhs, rhs);
}


// Difference: A \\ B = max(dA, -dB)
template <size_t Dim>
class DifferenceImplicitFunction : public ImplicitFunction<Dim>
{
public:
    using Point = typename ImplicitFunction<Dim>::Point;

    DifferenceImplicitFunction(std::shared_ptr<ImplicitFunction<Dim>> a,
        std::shared_ptr<ImplicitFunction<Dim>> b,
        bool smooth = false,
        float k = 0.0f)
        : m_a(a), m_b(b), m_smooth(smooth), m_k(k)
    {
        this->m_boundsMin = QVector3D(
            std::min(a->m_boundsMin.x(), b->m_boundsMin.x()),
            std::min(a->m_boundsMin.y(), b->m_boundsMin.y()),
            std::min(a->m_boundsMin.z(), b->m_boundsMin.z()));
        this->m_boundsMax = QVector3D(
            std::max(a->m_boundsMax.x(), b->m_boundsMax.x()),
            std::max(a->m_boundsMax.y(), b->m_boundsMax.y()),
            std::max(a->m_boundsMax.z(), b->m_boundsMax.z()));
    }

    float evaluateSDF(const Point& p) const override
    {
        float d1 = m_a->evaluateSDF(p);
        float d2 = m_b->evaluateSDF(p);
        if (m_smooth && m_k > 0.0f)
            return -smoothMinFloat(-d1, d2, m_k); // smooth max(d1, -d2)
        return std::max(d1, -d2);
    }

    inline const std::string buildUniforms() const override
    {
        std::string uA = m_a->buildUniforms();
        std::string uB = m_b->buildUniforms();

        std::string uniforms0 = ImplicitFunction<Dim>::buildUniforms();
        std::string uniforms1 = "uniform float " + this->m_uniformsNames.back() + ";\n";

        return uA + uB + uniforms0 + uniforms1;
    }

    std::string buildSDF() const override
    {
        std::string fA = m_a->buildSDF();
        std::string fB = m_b->buildSDF();

        std::string differenceSDF = "float " + this->m_dstFunctionName + "(vec3 p)\n"
            "{\n"
            "    float d1 = " + m_a->m_dstFunctionName + "(p);\n"
            "    float d2 = " + m_b->m_dstFunctionName + "(p);\n";
        if (m_smooth)
        {
            differenceSDF.append("    float k = " + this->m_uniformsNames.back() + ";\n");
            differenceSDF.append("    if (k <= 0.0) return max(d1, -d2);\n");
            differenceSDF.append("    float nd1 = -d1; float nd2 = d2;\n");
            differenceSDF.append("    float h = clamp(0.5 + 0.5*(nd2 - nd1)/k, 0.0, 1.0);\n");
            differenceSDF.append("    float s = mix(nd2, nd1, h) - k*h*(1.0 - h);\n");
            differenceSDF.append("    return -s;\n");
        }
        else
        {
            differenceSDF.append("    return max(d1, -d2);\n");
        }
        differenceSDF.append("}\n");

        return fA + fB + differenceSDF;
    }

    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        m_a->setGLUniforms(program);
        m_b->setGLUniforms(program);
        if (m_smooth)
            program.setUniformValue(this->m_uniformsNames.back().c_str(), m_k);
        ImplicitFunction<3>::setGLUniforms(program);
    }

private:
    std::shared_ptr<ImplicitFunction<Dim>> m_a, m_b;
    bool m_smooth = false;
    float m_k = 0.0f;
};

template <size_t Dim>
inline std::shared_ptr<ImplicitFunction<Dim>>
operator-(std::shared_ptr<ImplicitFunction<Dim>> lhs,
    std::shared_ptr<ImplicitFunction<Dim>> rhs)
{
    return std::make_shared<DifferenceImplicitFunction<Dim>>(lhs, rhs);
}