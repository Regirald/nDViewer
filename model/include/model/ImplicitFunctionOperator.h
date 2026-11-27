#pragma once
#include "ImplicitFunction.h"


static struct GLSLContext
{
    inline static int counter = 0;

    static std::string makeName(const std::string& base)
    {
        return base + "_" + std::to_string(counter++);
    }

    static std::string makeUniformUnique(std::string& uniforms, const std::string& base, const std::string& type)
    {
        std::string name = makeName(base);
        uniforms += "uniform " + type + " " + name + ";\n";
        return name;
    }

    static std::string makeUniform(std::string& uniforms, const std::string& base, const std::string& type)
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
class BinaryImplicitFunction : public ImplicitFunction<Dim>
{
public:
    using Point = typename ImplicitFunction<Dim>::Point;

    BinaryImplicitFunction(std::shared_ptr<ImplicitFunction<Dim>> a,
        std::shared_ptr<ImplicitFunction<Dim>> b,
        bool smooth,
        float k)
        : m_a(a->clone()), m_b(b->clone()), m_smooth(smooth), m_k(k)
    {
        initializeUniformNames();

        this->m_boundsMin = QVector3D(
            std::min(m_a->m_boundsMin.x(), m_b->m_boundsMin.x()),
            std::min(m_a->m_boundsMin.y(), m_b->m_boundsMin.y()),
            std::min(m_a->m_boundsMin.z(), m_b->m_boundsMin.z()));
        this->m_boundsMax = QVector3D(
            std::max(m_a->m_boundsMax.x(), m_b->m_boundsMax.x()),
            std::max(m_a->m_boundsMax.y(), m_b->m_boundsMax.y()),
            std::max(m_a->m_boundsMax.z(), m_b->m_boundsMax.z()));

        m_a->m_dstFunctionName = GLSLContext::makeName(m_a->m_dstFunctionName);
        m_b->m_dstFunctionName = GLSLContext::makeName(m_b->m_dstFunctionName);

        for (auto& name : m_a->m_uniformsNames)
            name = GLSLContext::makeName(name);

        for (auto& name : m_b->m_uniformsNames)
            name = GLSLContext::makeName(name);

    }

    inline const std::string buildUniforms() const override
    {
        return m_a->buildUniforms() + m_b->buildUniforms()
            + ImplicitFunction<Dim>::buildUniforms()
            + "uniform float " + this->m_uniformsNames.back() + ";\n";
    }

    inline const std::string buildSDF() const override
    {
        std::string sdf = m_a->buildSDF() + m_b->buildSDF();
        sdf += "float " + this->m_dstFunctionName + "(vec3 p)\n"
            "{\n"
            "    float d1 = " + m_a->m_dstFunctionName + "(p);\n"
            "    float d2 = " + m_b->m_dstFunctionName + "(p);\n";
        appendOperationSDF(sdf);
        sdf += "}\n";
        return sdf;
    }

    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        m_a->setGLUniforms(program);
        m_b->setGLUniforms(program);
        if (m_smooth)
            program.setUniformValue(this->m_uniformsNames.back().c_str(), m_k);
        ImplicitFunction<Dim>::setGLUniforms(program);
    }

protected:
    virtual void appendOperationSDF(std::string& sdf) const = 0;

    void initializeUniformNames() override
    {
        ImplicitFunction<Dim>::initializeUniformNames();
        this->m_uniformsNames.push_back("u_k");
    }

    std::shared_ptr<ImplicitFunction<Dim>> m_a, m_b;
    bool m_smooth = false;
    float m_k = 0.0f;
};

template <size_t Dim>
class UnionImplicitFunction : public BinaryImplicitFunction<Dim>
{
public:
    using Base = BinaryImplicitFunction<Dim>;
    using Point = typename Base::Point;

    std::shared_ptr<ImplicitFunction<Dim>> clone() const override
    {
        return std::make_shared<UnionImplicitFunction>(
            this->m_a, this->m_b, this->m_smooth, this->m_k);
    }

    UnionImplicitFunction(std::shared_ptr<ImplicitFunction<Dim>> a,
        std::shared_ptr<ImplicitFunction<Dim>> b,
        bool smooth = false,
        float k = 0.0f)
        : Base(a, b, smooth, k)
    {
    }

    float evaluateSDF(const Point& p) const override
    {
        float d1 = this->m_a->evaluateSDF(p);
        float d2 = this->m_b->evaluateSDF(p);
        if (this->m_smooth && this->m_k > 0.0f)
            return smoothMinFloat(d1, d2, this->m_k);
        return std::min(d1, d2);
    }

protected:
    void appendOperationSDF(std::string& sdf) const override
    {
        if (this->m_smooth)
        {
            sdf += "    float k = " + this->m_uniformsNames.back() + ";\n";
            sdf += "    if (k <= 0.0) return min(d1, d2);\n";
            sdf += "    float h = clamp(0.5 + 0.5*(d2 - d1)/k, 0.0, 1.0);\n";
            sdf += "    return mix(d2, d1, h) - k*h*(1.0 - h);\n";
        }
        else
            sdf += "    return min(d1, d2);\n";
    }
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
class IntersectionImplicitFunction : public BinaryImplicitFunction<Dim>
{
public:
    using Base = BinaryImplicitFunction<Dim>;
    using Point = typename Base::Point;

    std::shared_ptr<ImplicitFunction<Dim>> clone() const override
    {
        return std::make_shared<IntersectionImplicitFunction>(
            this->m_a, this->m_b, this->m_smooth, this->m_k);
    }

    IntersectionImplicitFunction(std::shared_ptr<ImplicitFunction<Dim>> a,
        std::shared_ptr<ImplicitFunction<Dim>> b,
        bool smooth = false,
        float k = 0.0f)
        : Base(a, b, smooth, k)
    {
    }

    float evaluateSDF(const Point& p) const override
    {
        float d1 = this->m_a->evaluateSDF(p);
        float d2 = this->m_b->evaluateSDF(p);
        if (this->m_smooth && this->m_k > 0.0f)
            return -smoothMinFloat(-d1, -d2, this->m_k);
        return std::max(d1, d2);
    }

protected:
    void appendOperationSDF(std::string& sdf) const override
    {
        if (this->m_smooth)
        {
            sdf += "    float k = " + this->m_uniformsNames.back() + ";\n";
            sdf += "    if (k <= 0.0) return max(d1, d2);\n";
            sdf += "    float nd1 = -d1; float nd2 = -d2;\n";
            sdf += "    float h = clamp(0.5 + 0.5*(nd2 - nd1)/k, 0.0, 1.0);\n";
            sdf += "    float s = mix(nd2, nd1, h) - k*h*(1.0 - h);\n";
            sdf += "    return -s;\n";
        }
        else
            sdf += "    return max(d1, d2);\n";
    }
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
class DifferenceImplicitFunction : public BinaryImplicitFunction<Dim>
{
public:
    using Base = BinaryImplicitFunction<Dim>;
    using Point = typename Base::Point;

    std::shared_ptr<ImplicitFunction<Dim>> clone() const override
    {
        return std::make_shared<DifferenceImplicitFunction>(
            this->m_a, this->m_b, this->m_smooth, this->m_k);
    }

    DifferenceImplicitFunction(std::shared_ptr<ImplicitFunction<Dim>> a,
        std::shared_ptr<ImplicitFunction<Dim>> b,
        bool smooth = false,
        float k = 0.0f)
        : Base(a, b, smooth, k)
    {
    }

    float evaluateSDF(const Point& p) const override
    {
        float d1 = this->m_a->evaluateSDF(p);
        float d2 = this->m_b->evaluateSDF(p);
        if (this->m_smooth && this->m_k > 0.0f)
            return -smoothMinFloat(-d1, d2, this->m_k);
        return std::max(d1, -d2);
    }

protected:
    void appendOperationSDF(std::string& sdf) const override
    {
        if (this->m_smooth)
        {
            sdf += "    float k = " + this->m_uniformsNames.back() + ";\n";
            sdf += "    if (k <= 0.0) return max(d1, -d2);\n";
            sdf += "    float nd1 = -d1; float nd2 = d2;\n";
            sdf += "    float h = clamp(0.5 + 0.5*(nd2 - nd1)/k, 0.0, 1.0);\n";
            sdf += "    float s = mix(nd2, nd1, h) - k*h*(1.0 - h);\n";
            sdf += "    return -s;\n";
        }
        else
            sdf += "    return max(d1, -d2);\n";
    }
};

template <size_t Dim>
inline std::shared_ptr<ImplicitFunction<Dim>>
operator-(std::shared_ptr<ImplicitFunction<Dim>> lhs,
    std::shared_ptr<ImplicitFunction<Dim>> rhs)
{
    return std::make_shared<DifferenceImplicitFunction<Dim>>(lhs, rhs);
}