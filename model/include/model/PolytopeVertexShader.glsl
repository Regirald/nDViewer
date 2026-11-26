R"POLY_VTX_SHADER(
#version 330 core

#extension GL_ARB_shader_storage_buffer_object : require

layout(location = 0) in vec3 a_cubePosition;

// Camera
uniform mat4 u_viewProjection;

// Sphere selection
uniform int u_currentSphereIndex;
uniform float u_radius;

// Sphere data
layout(std430, binding = 0) readonly buffer VerticeSetData
{
    vec4 u_vertices[];
};

void main()
{
    vec4 vertex = u_vertices[u_currentSphereIndex];

    // Reconstruct sphere center
    vec3 center = vertex.xyz / (1.0 - 0.3 * vertex.w);

    // Bounding cube around the sphere
    vec3 worldPosition = center + a_cubePosition * u_radius;

    gl_Position = u_viewProjection * vec4(worldPosition, 1.0);
}

)POLY_VTX_SHADER"