R"POLY_VTX_SHADER(
#version 330 core

#extension GL_ARB_shader_storage_buffer_object : require

layout(location = 0) in vec2 a_quadPosition;
noperspective out vec2 v_uv;

// Camera
uniform mat4 u_viewProjection;
    uniform mat4 u_view;
    uniform mat4 u_projection;
uniform mat4 u_invView;

// Sphere selection
uniform int u_currentSphereIndex;
uniform float u_radius;

// Sphere data
layout(std430, binding = 0) buffer VerticeSetData
{
    vec4 u_vertices[];
};

void main()
{
    vec4 vertex = u_vertices[u_currentSphereIndex];
    vec3 centerView = (u_view * vec4(vertex.xyz, 1.0)).xyz;
    float depth = -centerView.z;
    float radiusSquared = u_radius * u_radius;
    float denominator = depth * depth - radiusSquared;

    if (denominator <= 0.0001)
    {
        gl_Position = vec4(a_quadPosition, -1.0, 1.0);
        v_uv = a_quadPosition * 0.5 + 0.5;
        return;
    }

    float xDiscriminant = sqrt(max(0.0, centerView.x * centerView.x + depth * depth - radiusSquared));
    float yDiscriminant = sqrt(max(0.0, centerView.y * centerView.y + depth * depth - radiusSquared));
    float xMin = (centerView.x * depth - u_radius * xDiscriminant) / denominator;
    float xMax = (centerView.x * depth + u_radius * xDiscriminant) / denominator;
    float yMin = (centerView.y * depth - u_radius * yDiscriminant) / denominator;
    float yMax = (centerView.y * depth + u_radius * yDiscriminant) / denominator;

    vec4 xMinClip = u_projection * vec4(xMin, 0.0, -1.0, 1.0);
    vec4 xMaxClip = u_projection * vec4(xMax, 0.0, -1.0, 1.0);
    vec4 yMinClip = u_projection * vec4(0.0, yMin, -1.0, 1.0);
    vec4 yMaxClip = u_projection * vec4(0.0, yMax, -1.0, 1.0);

    float ndcMinX = xMinClip.x / xMinClip.w;
    float ndcMaxX = xMaxClip.x / xMaxClip.w;
    float ndcMinY = yMinClip.y / yMinClip.w;
    float ndcMaxY = yMaxClip.y / yMaxClip.w;

    vec2 ndc = vec2(mix(ndcMinX, ndcMaxX, a_quadPosition.x * 0.5 + 0.5), mix(ndcMinY, ndcMaxY, a_quadPosition.y * 0.5 + 0.5));
    gl_Position = vec4(ndc, -1.0, 1.0);
    v_uv = ndc * 0.5 + 0.5;
}

)POLY_VTX_SHADER"