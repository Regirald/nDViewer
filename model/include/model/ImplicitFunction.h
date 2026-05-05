#pragma once
#include "core/Vec.h"
#include <string>

// forward-declare Qt class to avoid forcing a Qt dependency here
class QOpenGLShaderProgram;

template <size_t Dim>
class ImplicitFunction {
public:
    using Point = Vec<Dim, float>;

    virtual float evaluateSDF(const Point& p) const = 0;

    virtual const std::string getFragmentShader() const;
    virtual const std::string getGLSL_SDF() const;
    virtual const std::string getGLSL_parameters() const;
    virtual void setGLUniforms(QOpenGLShaderProgram& program) const;
};

template <size_t Dim>
inline const std::string ImplicitFunction<Dim>::getGLSL_SDF() const
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
inline void ImplicitFunction<Dim>::setGLUniforms(QOpenGLShaderProgram& /*program*/) const
{
    // no-op default
}

template <size_t Dim>
inline const std::string ImplicitFunction<Dim>::getFragmentShader() const
{
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

        vec3 worldToTex(vec3 p)
        {
            return (p - u_boundsMin) / (u_boundsMax - u_boundsMin);
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
                    vec3 lightDir = normalize(vec3(1,1,1));

                    float diff = max(dot(n, lightDir), 0.0);
                    FragColor = vec4(vec3(diff), 1.0);

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

    return std::string(part1) + getGLSL_SDF() + std::string(part2);
}
