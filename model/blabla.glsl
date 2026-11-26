
#version 330 core


noperspective in vec2 v_uv;

// Camera
uniform vec3 u_cameraPos;
uniform vec3 u_lightDir;
uniform vec3 u_face_normal;
uniform mat4 u_view;
uniform mat4 u_invView;
uniform mat4 u_projection;
uniform mat4 u_invProjection;

// Sphere selection
uniform int u_currentSphereIndex;
uniform float u_radius;

// Sphere data
#extension GL_ARB_shader_storage_buffer_object : require
layout(std430, binding = 0) buffer VerticeSetData
{
    vec4 u_vertices[];
};

// --------------------------------------------------
// Sphere center
// --------------------------------------------------

vec3 getSphereCenter()
{
    vec4 vertex = u_vertices[u_currentSphereIndex];

    return vertex.xyz / (1.0 - 0.3 * vertex.w);
}


// --------------------------------------------------
// Ray / sphere intersection
// --------------------------------------------------

bool intersectSphere(
    vec3 ro,
    vec3 rd,
    vec3 center,
    float radius,
    out float t
)
{
    vec3 oc = ro - center;

    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;

    float h = b * b - c;

    if (h < 0.0)
        return false;

    float s = sqrt(h);

    float tNear = -b - s;
    float tFar  = -b + s;

    // Camera outside or inside the sphere
    t = (tNear > 0.0) ? tNear : tFar;

    return t > 0.0;
}


// --------------------------------------------------
// Ray reconstruction
// --------------------------------------------------

vec3 reconstructRayDirection()
{
	vec2 ndc = v_uv * 2.0 - 1.0;

    vec4 rayClip = vec4(ndc, -1.0, 1.0);

    vec4 rayView = u_invProjection * rayClip;

    rayView /= rayView.w;

    return normalize(
        (u_invView * vec4(rayView.xyz, 0.0)).xyz
    );
}


// --------------------------------------------------
// Sphere shading
// --------------------------------------------------

vec4 shadeSphere(vec3 p, vec3 center, vec3 rayDir)
{
    vec3 normal = normalize(p - center);

    vec3 reflected = reflect(rayDir, normal);

    float intensity = clamp(
        dot(u_lightDir, reflected),
        0.0,
        1.0
    );

    vec3 white = vec3(0.85);
    vec3 yellow = vec3(0.75, 0.75, 0.0);

    vec3 color = mix(yellow, white, intensity);

    return vec4(color, 1.0);
}


// --------------------------------------------------
// Main
// --------------------------------------------------

void main()
{
    vec3 center = getSphereCenter();

    vec3 rayOrigin = u_cameraPos;
    vec3 rayDir = reconstructRayDirection();

    // Exact ray / sphere intersection
    float t;

    if (!intersectSphere(rayOrigin, rayDir, center, u_radius, t))
    {
        gl_FragColor = vec4(1.0)
    }
    else
    {
    // Exact intersection position
    vec3 hitPosition = rayOrigin + t * rayDir;

    // Project the actual sphere intersection
    vec4 clip = u_projection * u_view * vec4(hitPosition, 1.0);

    float ndcDepth = clip.z / clip.w;

    float depth = ndcDepth * 0.5 + 0.5;

    if (depth < 0.0 || depth > 1.0)
        discard;

    // Write actual sphere depth, not bounding cube depth
    gl_FragDepth = depth;

    gl_FragColor = shadeSphere(hitPosition, center, rayDir);
    }
}
