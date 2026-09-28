#pragma once
#include "ImplicitFunction.h"

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
        : m_a(a), m_b(b), m_smooth(smooth), m_k(k) {
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
            return smoothMinFloat(d1, d2, m_k);
        return std::min(d1, d2);
    }

    std::string buildGLSL(GLSLContext& ctx) const override
    {
        std::string fA = m_a->buildGLSL(ctx);
        std::string fB = m_b->buildGLSL(ctx);

        std::string fname = ctx.makeName("sdf_union");

        ctx.append("float " + fname + "(vec3 p) {");
        ctx.append("    float d1 = " + fA + "(p);");
        ctx.append("    float d2 = " + fB + "(p);");
        if (m_smooth)
        {
            u_k = ctx.makeUniformUnique("u_k", "float");
            ctx.append("    float k = " + u_k + ";");
            ctx.append("    if (k <= 0.0) return min(d1, d2);");
            ctx.append("    float h = clamp(0.5 + 0.5*(d2 - d1)/k, 0.0, 1.0);");
            ctx.append("    return mix(d2, d1, h) - k*h*(1.0 - h);");
        }
        else
        {
            ctx.append("    return min(d1, d2);");
        }
        ctx.append("}");

        return fname;
    }
    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        m_a->setGLUniforms(program);
        m_b->setGLUniforms(program);
        if (m_smooth)
            program.setUniformValue(u_k.c_str(), m_k);
        ImplicitFunction<3>::setGLUniforms(program); //overwrite the boundaries

    }

private:
    std::shared_ptr<ImplicitFunction<Dim>> m_a, m_b;
    bool m_smooth = false;
    float m_k = 0.0f;
    mutable std::string u_k;
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
    }

    float evaluateSDF(const Point& p) const override
    {
        float d1 = m_a->evaluateSDF(p);
        float d2 = m_b->evaluateSDF(p);
        if (m_smooth && m_k > 0.0f)
            return -smoothMinFloat(-d1, -d2, m_k); // smooth max
        return std::max(d1, d2);
    }

    std::string buildGLSL(GLSLContext& ctx) const override
    {
        std::string fA = m_a->buildGLSL(ctx);
        std::string fB = m_b->buildGLSL(ctx);

        std::string fname = ctx.makeName("sdf_intersection");

        ctx.append("float " + fname + "(vec3 p) {");
        ctx.append("    float d1 = " + fA + "(p);");
        ctx.append("    float d2 = " + fB + "(p);");
        if (m_smooth)
        {
            u_k = ctx.makeUniformUnique("u_k", "float");
            ctx.append("    float k = " + u_k + ";");
            ctx.append("    if (k <= 0.0) return max(d1, d2);");
            ctx.append("    float nd1 = -d1; float nd2 = -d2;");
            ctx.append("    float h = clamp(0.5 + 0.5*(nd2 - nd1)/k, 0.0, 1.0);");
            ctx.append("    float s = mix(nd2, nd1, h) - k*h*(1.0 - h);");
            ctx.append("    return -s;");
        }
        else
        {
            ctx.append("    return max(d1, d2);");
        }
        ctx.append("}");

        return fname;
    }

    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        m_a->setGLUniforms(program);
        m_b->setGLUniforms(program);
        if (m_smooth)
            program.setUniformValue(u_k.c_str(), m_k);
        ImplicitFunction<3>::setGLUniforms(program);
    }

private:
    std::shared_ptr<ImplicitFunction<Dim>> m_a, m_b;
    bool m_smooth = false;
    float m_k = 0.0f;
    mutable std::string u_k;
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

    std::string buildGLSL(GLSLContext& ctx) const override
    {
        std::string fA = m_a->buildGLSL(ctx);
        std::string fB = m_b->buildGLSL(ctx);

        std::string fname = ctx.makeName("sdf_difference");

        ctx.append("float " + fname + "(vec3 p) {");
        ctx.append("    float d1 = " + fA + "(p);");
        ctx.append("    float d2 = " + fB + "(p);");
        if (m_smooth)
        {
            u_k = ctx.makeUniformUnique("u_k", "float");
            ctx.append("    float k = " + u_k + ";");
            ctx.append("    if (k <= 0.0) return max(d1, -d2);");
            ctx.append("    float nd1 = -d1; float nd2 = d2;");
            ctx.append("    float h = clamp(0.5 + 0.5*(nd2 - nd1)/k, 0.0, 1.0);");
            ctx.append("    float s = mix(nd2, nd1, h) - k*h*(1.0 - h);");
            ctx.append("    return -s;");
        }
        else
        {
            ctx.append("    return max(d1, -d2);");
        }
        ctx.append("}");

        return fname;
    }

    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        m_a->setGLUniforms(program);
        m_b->setGLUniforms(program);
        if (m_smooth)
            program.setUniformValue(u_k.c_str(), m_k);
        ImplicitFunction<3>::setGLUniforms(program);
    }

private:
    std::shared_ptr<ImplicitFunction<Dim>> m_a, m_b;
    bool m_smooth = false;
    float m_k = 0.0f;
    mutable std::string u_k;
};

template <size_t Dim>
inline std::shared_ptr<ImplicitFunction<Dim>>
operator-(std::shared_ptr<ImplicitFunction<Dim>> lhs,
    std::shared_ptr<ImplicitFunction<Dim>> rhs)
{
    return std::make_shared<DifferenceImplicitFunction<Dim>>(lhs, rhs);
}