R"POLY_VTX_SHADER(
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
uniform bool u_isEdge;

// Vertex-set data
layout(std430, binding = 0) buffer VerticeSetData
{
    vec4 u_vertices[];
};

// Edge endpoint data: two consecutive vec4 values per edge
layout(std430, binding = 1) buffer PolytopeEdgeData
{
    vec4 u_edgeVertices[];
};

void main()
{
    vec3 centerWorld;
    float boundingRadius = u_radius;

    if (u_isEdge)
    {
        int edgeOffset = u_currentSphereIndex * 2;
        vec3 a = u_edgeVertices[edgeOffset].xyz;
        vec3 b = u_edgeVertices[edgeOffset + 1].xyz;
        vec3 halfEdge = (b - a) * 0.5;

        centerWorld = (a + b) * 0.5;
        boundingRadius = sqrt(dot(halfEdge, halfEdge) + u_radius * u_radius);
    }
    else
    {
        centerWorld = u_vertices[u_currentSphereIndex].xyz;
    }

    // Direction from camera to sphere center
    vec3 viewDir = normalize(centerWorld - u_cameraPos);

    float d = length(centerWorld - u_cameraPos);
    float r = boundingRadius;

    if (d - r < 0.0) // Camera is inside the sphere -> quad go full scree
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

    gl_Position = u_viewProjection * vec4(worldPosition, 1.0);

    v_uv = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
}

)POLY_VTX_SHADER"