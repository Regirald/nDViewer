
#version 330 core

#extension GL_ARB_shader_storage_buffer_object : require

layout(location = 0) in vec2 a_quadPosition;
noperspective out vec2 v_uv;

// Camera
uniform mat4 u_viewProjection;
uniform mat4 u_invView;
uniform vec3 u_cameraPos;


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

    vec3 centerWorld = vertex.xyz;

    vec3 toCenter = centerWorld - u_cameraPos;
    float d = length(toCenter);

    // Prevent invalid geometry when the camera enters the sphere.
    float safeD = max(d, u_radius * 1.001);

    // Perspective-correct billboard radius.
    float billboardRadius = u_radius * safeD / sqrt(safeD * safeD - u_radius * u_radius);

    // Camera-facing basis.
    vec3 right = u_invView[0].xyz;
    vec3 up = u_invView[1].xyz;

    vec3 worldPosition = centerWorld + (right * a_quadPosition.x + up * a_quadPosition.y) * billboardRadius;

    gl_Position = u_viewProjection * vec4(worldPosition, 1.0);

    v_uv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;

