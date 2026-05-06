#pragma once
#include "core/Vec.h"
#include <string>
#include <algorithm>
#include <QOpenGLShaderProgram>

struct GLSLContext
{
    int counter = 0;
    std::string code;
    std::string uniforms;

    std::unordered_map<std::string, std::string> uniformMap;

    std::string makeName(const std::string& base)
    {
        return base + "_" + std::to_string(counter++);
    }

    std::string makeUniform(const std::string& base, const std::string& type)
    {
        std::string name = base + "_" + std::to_string(counter++);
        uniforms += "uniform " + type + " " + name + ";\n";
        return name;
    }

    void append(const std::string& s)
    {
        code += s + "\n";
    }
};
// forward-declare Qt class to avoid forcing a Qt dependency here
class QOpenGLShaderProgram;

template <size_t Dim>
class ImplicitFunction {
public:
    using Point = Vec<Dim, float>;

    virtual float evaluateSDF(const Point& p) const = 0;

    virtual const std::string getFragmentShader() const;
    virtual std::string buildGLSL(GLSLContext& ctx) const;
    virtual const std::string getGLSL_parameters() const;
    virtual void setGLUniforms(QOpenGLShaderProgram& program) const;

    QVector3D m_boundsMin;
    QVector3D m_boundsMax;
};



template <size_t Dim>
inline std::string ImplicitFunction<Dim>::buildGLSL(GLSLContext& ctx) const
{
    const char* sdf = R"(
        float sampleSDF(vec3 p)
            {
                return 0.f;
            }
        )";
    return std::string(sdf);
};


template <size_t Dim>
inline const std::string ImplicitFunction<Dim>::getGLSL_parameters() const
{
    return std::string();
}

template <size_t Dim>
inline void ImplicitFunction<Dim>::setGLUniforms(QOpenGLShaderProgram& m_program) const
{
    m_program.setUniformValue("u_boundsMin", m_boundsMin);
    m_program.setUniformValue("u_boundsMax", m_boundsMax);
}
template <size_t Dim>
class UnionImplicitFunction : public ImplicitFunction<Dim>
{
public:
    using Point = typename ImplicitFunction<Dim>::Point;

    UnionImplicitFunction(std::shared_ptr<ImplicitFunction<Dim>> a,
        std::shared_ptr<ImplicitFunction<Dim>> b)
        : m_a(a), m_b(b) {
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
        return std::min(m_a->evaluateSDF(p), m_b->evaluateSDF(p));
    }

    std::string buildGLSL(GLSLContext& ctx) const override
    {
        std::string fA = m_a->buildGLSL(ctx);
        std::string fB = m_b->buildGLSL(ctx);

        std::string fname = ctx.makeName("sdf_union");

        ctx.append("float " + fname + "(vec3 p) {");
        ctx.append("    float d1 = " + fA + "(p);");
        ctx.append("    float d2 = " + fB + "(p);");
        ctx.append("    return min(d1, d2);");
        ctx.append("}");

        return fname;
    }
    void setGLUniforms(QOpenGLShaderProgram& program) const override
    {
        m_a->setGLUniforms(program);
        m_b->setGLUniforms(program);
        ImplicitFunction<3>::setGLUniforms(program); //overwrite the boundaries

    }

private:
    std::shared_ptr<ImplicitFunction<Dim>> m_a, m_b;
};

template <size_t Dim>
inline std::shared_ptr<ImplicitFunction<Dim>>
operator+(std::shared_ptr<ImplicitFunction<Dim>> lhs,
    std::shared_ptr<ImplicitFunction<Dim>> rhs)
{
    return std::make_shared<UnionImplicitFunction<Dim>>(lhs, rhs);
}

template <size_t Dim>
inline const std::string ImplicitFunction<Dim>::getFragmentShader() const
{
    GLSLContext ctx;

    std::string rootFunc = buildGLSL(ctx);

    std::string sdfEntry =
        "float sampleSDF(vec3 p)\n"
        "{\n"
        "    return " + rootFunc + "(p);\n"
        "}\n";

    const char* part1 = R"(
        #version 330 core

        in vec2 v_uv;
        out vec4 FragColor;

        uniform vec3 u_cameraPos;
        uniform mat4 u_view;
        uniform mat4 u_invView;
        uniform mat4 u_projection;
        uniform mat4 u_invProjection;

        uniform vec3 u_boundsMin;
        uniform vec3 u_boundsMax;

        uniform float u_maxDistance;
        uniform int u_maxSteps;
        uniform float u_epsilon;

        // -------------------------
        // AABB intersection
        // -------------------------
        bool intersectAABB(vec3 ro, vec3 rd, out float tmin, out float tmax)
        {
            vec3 invDir = 1.0 / rd;
            vec3 t0 = (u_boundsMin - ro) * invDir;
            vec3 t1 = (u_boundsMax - ro) * invDir;

            vec3 tsmaller = min(t0, t1);
            vec3 tbigger  = max(t0, t1);

            tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
            tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

            return tmax > max(tmin, 0.0);
        }
)";

    const char* part2 = R"(

        
        vec3 estimateNormal(vec3 p)
        {
            float e = 0.002;

            return normalize(vec3(
                sampleSDF(p + vec3(e,0,0)) - sampleSDF(p - vec3(e,0,0)),
                sampleSDF(p + vec3(0,e,0)) - sampleSDF(p - vec3(0,e,0)),
                sampleSDF(p + vec3(0,0,e)) - sampleSDF(p - vec3(0,0,e))
            ));
        }

        void main()
        {
            vec2 ndc = v_uv * 2.0 - 1.0;

            vec4 rayClip = vec4(ndc, -1.0, 1.0);
            vec4 rayView = u_invProjection * rayClip;
            rayView = vec4(rayView.xyz, 0.0);

            vec3 rayDir = normalize((u_invView * rayView).xyz);
            vec3 rayOrigin = u_cameraPos;

            float tmin, tmax;
            if (!intersectAABB(rayOrigin, rayDir, tmin, tmax))
            {
                discard;
            }

            float t = max(tmin, 0.0);

            for (int i = 0; i < u_maxSteps; ++i)
            {
                vec3 p = rayOrigin + t * rayDir;
                float d = sampleSDF(p);

                if (abs(d) < u_epsilon)
                {
                    vec3 n = estimateNormal(p);            
                    vec3 color = 0.5 + 0.5 * abs(n);                   
                    float alpha = 0.9;
                    FragColor = vec4(color, alpha);

                    // ---- Depth write ----
                    vec4 clip = u_projection * u_view * vec4(p, 1.0);
                    float ndcDepth = clip.z / clip.w;
                    gl_FragDepth = ndcDepth * 0.5 + 0.5;
                    return;
                }

                t += max(abs(d) * 0.7, 0.001);

                if (t > tmax || t > u_maxDistance)
                    break;
            }

            discard;

        }
)";

    return 
        std::string(part1) 
        + ctx.uniforms
        + ctx.code
        + sdfEntry 
        + std::string(part2);
}
