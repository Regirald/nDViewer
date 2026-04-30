#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <vector>
#include "renderer/RenderImplicitFunction.h"

template<size_t Dim>
RenderImplicitFunction<Dim>::RenderImplicitFunction(ImplicitFunction<Dim>* implicitFunction)
    : m_implicitFunction(implicitFunction)
{
}

template<size_t Dim>
void RenderImplicitFunction<Dim>::initializeGLResources()
{
    if (!m_implicitFunction)
        return;

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    gl = ctx->extraFunctions();
    gl->initializeOpenGLFunctions();

    // -------------------------
    // Generate SDF texture
    // -------------------------
    generateSDF();

    // -------------------------
    // Compile shaders
    // -------------------------
    if (!m_program.isLinked())
    {
        const char* vs = R"(
#version 330 core

layout(location = 0) in vec2 a_position;
out vec2 v_uv;

void main()
{
    v_uv = a_position * 0.5 + 0.5;
    gl_Position = vec4(a_position, 0.0, 1.0);
}
)";

        const char* fs = R"(
#version 330 core

in vec2 v_uv;
out vec4 FragColor;

uniform sampler3D u_sdf;

uniform mat4 u_invView;
uniform mat4 u_invProjection;
uniform vec3 u_cameraPos;

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

// -------------------------
vec3 worldToTex(vec3 p)
{
    return (p - u_boundsMin) / (u_boundsMax - u_boundsMin);
}

float sampleSDF(vec3 p)
{
    vec3 uvw = worldToTex(p);

    if (any(lessThan(uvw, vec3(0.0))) || any(greaterThan(uvw, vec3(1.0))))
        return 1e6;

    return texture(u_sdf, uvw).r;
}

vec3 estimateNormal(vec3 p)
{
    float e = 0.002;

    return normalize(vec3(
        sampleSDF(p + vec3(e,0,0)) - sampleSDF(p - vec3(e,0,0)),
        sampleSDF(p + vec3(0,e,0)) - sampleSDF(p - vec3(0,e,0)),
        sampleSDF(p + vec3(0,0,e)) - sampleSDF(p - vec3(0,0,e))
    ));
}

// -------------------------
void main()
{
    // Reconstruct ray
    vec2 ndc = v_uv * 2.0 - 1.0;

    vec4 rayClip = vec4(ndc, -1.0, 1.0);
    vec4 rayView = u_invProjection * rayClip;
    rayView = vec4(rayView.xyz, 0.0);

    vec3 rayDir = normalize((u_invView * rayView).xyz);
    vec3 rayOrigin = u_cameraPos;

    // Intersect bounding box
    float tmin, tmax;
    if (!intersectAABB(rayOrigin, rayDir, tmin, tmax))
    {
        FragColor = vec4(0.0);
        return;
    }

    float t = max(tmin, 0.0);

    // Raymarch
    for (int i = 0; i < u_maxSteps; ++i)
    {
        vec3 p = rayOrigin + t * rayDir;
        float d = sampleSDF(p);

        if (d < u_epsilon)
        {
            vec3 n = estimateNormal(p);
            vec3 lightDir = normalize(vec3(1,1,1));

            float diff = max(dot(n, lightDir), 0.0);
            FragColor = vec4(vec3(diff), 1.0);
            return;
        }

        // Robust stepping (handles non-perfect SDF)
        t += max(d * 0.7, 0.001);

        if (t > tmax || t > u_maxDistance)
            break;
    }

    FragColor = vec4(0.0);
}
)";

        m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
        m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
        m_program.link();
    }

    // -------------------------
    // Fullscreen quad
    // -------------------------
    if (!m_fullscreenVAO)
    {
        float quad[] = {
            -1.f, -1.f,
             1.f, -1.f,
            -1.f,  1.f,
             1.f,  1.f
        };

        gl->glGenVertexArrays(1, &m_fullscreenVAO);
        gl->glGenBuffers(1, &m_fullscreenVBO);

        gl->glBindVertexArray(m_fullscreenVAO);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenVBO);
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

        gl->glEnableVertexAttribArray(0);
        gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        gl->glBindVertexArray(0);
    }

    m_initialized = true;
}

template<size_t Dim>
void RenderImplicitFunction<Dim>::generateSDF()
{
    const int N = m_resolution;
    std::vector<float> data(N * N * N);

    QVector3D size = m_boundsMax - m_boundsMin;

    for (int z = 0; z < N; ++z)
        for (int y = 0; y < N; ++y)
            for (int x = 0; x < N; ++x)
            {
                QVector3D uvw(
                    float(x) / (N - 1),
                    float(y) / (N - 1),
                    float(z) / (N - 1)
                );

                QVector3D p = m_boundsMin + QVector3D(
                    uvw.x() * size.x(),
                    uvw.y() * size.y(),
                    uvw.z() * size.z()
                );

                float d = 0.f;

                if constexpr (Dim == 2)
                    d = m_implicitFunction->evaluate({ p.x(), p.y() });
                else
                    d = m_implicitFunction->evaluate({ p.x(), p.y(), p.z() });

                data[x + y * N + z * N * N] = d;
            }

    uploadSDF(data);
}

template<size_t Dim>
void RenderImplicitFunction<Dim>::uploadSDF(const std::vector<float>& data)
{
    if (!m_sdfTexture)
        gl->glGenTextures(1, &m_sdfTexture);

    gl->glBindTexture(GL_TEXTURE_3D, m_sdfTexture);

    gl->glTexImage3D(
        GL_TEXTURE_3D,
        0,
        GL_R32F,
        m_resolution,
        m_resolution,
        m_resolution,
        0,
        GL_RED,
        GL_FLOAT,
        data.data()
    );

    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

template<size_t Dim>
void RenderImplicitFunction<Dim>::render(const ndv::RenderContext& context)
{
    if (!m_initialized)
        initializeGLResources();

    if (!m_initialized)
        return;

    m_program.bind();

    // Texture
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_3D, m_sdfTexture);
    m_program.setUniformValue("u_sdf", 0);

    // Bounds
    m_program.setUniformValue("u_boundsMin", m_boundsMin);
    m_program.setUniformValue("u_boundsMax", m_boundsMax);

    // Camera
    m_program.setUniformValue("u_invView", context.view.inverted());
    m_program.setUniformValue("u_invProjection", context.projection.inverted());
    m_program.setUniformValue("u_cameraPos", context.cameraPosition);

    // Parameters
    m_program.setUniformValue("u_maxDistance", 100.0f);
    m_program.setUniformValue("u_maxSteps", 128);
    m_program.setUniformValue("u_epsilon", 0.001f);

    // Draw fullscreen
    gl->glDisable(GL_DEPTH_TEST);
    gl->glBindVertexArray(m_fullscreenVAO);
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl->glBindVertexArray(0);

    m_program.release();
}

template class RenderImplicitFunction<2>;
template class RenderImplicitFunction<3>;