
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
    vec3 centerWorld = u_vertices[u_currentSphereIndex].xyz;

    // Direction from camera to sphere center
    vec3 viewDir = normalize(centerWorld - u_cameraPos);

    float d = length(centerWorld - u_cameraPos);
    float r = u_radius;

    if (d - r < 0.0)
    {
        gl_Position = vec4(a_quadPosition, 0.0, 1.0);
        v_uv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
        return; 
    }

    d = max(d, r * 1.01);
    float billboardRadius = r * d / sqrt(d * d - r * r);

    // Camera-facing orthonormal basis
    vec3 right = normalize(cross(viewDir, normalize(u_invView[1].xyz)));
    vec3 up = normalize(cross(right, viewDir));

    // Construct billboard vertex
    vec3 worldPosition = centerWorld + (right * a_quadPosition.x + up * a_quadPosition.y) * billboardRadius;
    if (inside)
        worldPosition = viewDir * billboardRadius;

    gl_Position = u_viewProjection * vec4(worldPosition, 1.0);

    v_uv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
}

