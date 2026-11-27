R"POLY_FRAG_SHADER(
#version 330 core

#extension GL_ARB_shader_storage_buffer_object : require

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

// --------------------------------------------------
// Sphere center
// --------------------------------------------------

vec3 getSphereCenter()
{
    vec4 vertex = u_isEdge
        ? u_edgeVertices[u_currentSphereIndex * 2]
        : u_vertices[u_currentSphereIndex];

    return vertex.xyz;
}

void getEdgeEndpoints(out vec3 a, out vec3 b)
{
    int edgeOffset = u_currentSphereIndex * 2;
    a = u_edgeVertices[edgeOffset].xyz;
    b = u_edgeVertices[edgeOffset + 1].xyz;
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

    bool inside = c < 0.0;

    // Outside: front surface
    // Inside: exit surface
    t = inside ? tFar : tNear;

    return t > 0.0;
}



// --------------------------------------------------
// Ray / capsule intersection
// --------------------------------------------------

bool intersectSphereAt(
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
    float tFar = -b + s;
    t = tNear > 0.0 ? tNear : tFar;
    return t > 0.0;
}

bool intersectCapsule(
    vec3 ro,
    vec3 rd,
    vec3 a,
    vec3 b,
    float radius,
    out float t,
    out vec3 normal
)
{
    vec3 ba = b - a;
    vec3 oa = ro - a;
    float baba = dot(ba, ba);
    float bestT = 1e30;
    vec3 bestNormal = vec3(0.0);
    bool hit = false;

    if (baba > 1e-8)
    {
        float bard = dot(ba, rd);
        float baoa = dot(ba, oa);
        float rdoa = dot(rd, oa);
        float oaoa = dot(oa, oa);
        float qa = baba - bard * bard;
        float qb = baba * rdoa - baoa * bard;
        float qc = baba * oaoa - baoa * baoa - radius * radius * baba;

        if (abs(qa) > 1e-8)
        {
            float h = qb * qb - qa * qc;
            if (h >= 0.0)
            {
                float sqrtH = sqrt(h);
                float t0 = (-qb - sqrtH) / qa;
                float t1 = (-qb + sqrtH) / qa;

                for (int i = 0; i < 2; ++i)
                {
                    float candidateT = i == 0 ? t0 : t1;
                    float y = baoa + candidateT * bard;
                    if (candidateT > 0.0 && y >= 0.0 && y <= baba && candidateT < bestT)
                    {
                        vec3 hitPoint = ro + candidateT * rd;
                        vec3 axisPoint = a + (y / baba) * ba;
                        bestT = candidateT;
                        bestNormal = normalize(hitPoint - axisPoint);
                        hit = true;
                    }
                }
            }
        }
    }

    float capT;
    if (intersectSphereAt(ro, rd, a, radius, capT) && capT < bestT)
    {
        bestT = capT;
        bestNormal = normalize(ro + capT * rd - a);
        hit = true;
    }

    if (intersectSphereAt(ro, rd, b, radius, capT) && capT < bestT)
    {
        bestT = capT;
        bestNormal = normalize(ro + capT * rd - b);
        hit = true;
    }

    if (!hit)
        return false;

    t = bestT;
    normal = bestNormal;
    return true;
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

    return normalize((u_invView * vec4(rayView.xyz, 0.0)).xyz);
}


// --------------------------------------------------
// Sphere shading
// --------------------------------------------------

vec4 shadeSurface(vec3 normal, vec3 rayDir)
{
    vec3 reflected = reflect(rayDir, normal);

    float intensity = clamp(dot(u_lightDir, reflected), 0.0, 1.0);

    vec3 white = vec3(0.85);
    vec3 yellow = vec3(0.75, 0.75, 0.0);

    vec3 color = mix(yellow, white, intensity);

    return vec4(color, 1.0);
}

vec4 shadeSphere(vec3 p, vec3 center, vec3 rayDir)
{
    return shadeSurface(normalize(p - center), rayDir);
}

vec4 shadeEdge(vec3 normal, vec3 rayDir)
{
    return shadeSurface(normal, rayDir);
}


// --------------------------------------------------
// Main
// --------------------------------------------------

void main()
{
    vec3 center = getSphereCenter();

    vec3 rayOrigin = u_cameraPos;
    vec3 rayDir = reconstructRayDirection();

    // Intersect the actual primitive represented by this quad.
    float t;
    vec3 normal;

    if (u_isEdge)
    {
        vec3 a;
        vec3 b;
        getEdgeEndpoints(a, b);
        if (!intersectCapsule(rayOrigin, rayDir, a, b, u_radius, t, normal))
        {
            gl_FragColor = vec4(1.0);
            return;
        }
    }
    else
    {
        if (!intersectSphere(rayOrigin, rayDir, center, u_radius, t))
        {
            discard;
        }
        normal = normalize(rayOrigin + t * rayDir - center);
    }

    vec3 hitPosition = rayOrigin + t * rayDir;

    // Project the actual sphere intersection
    vec4 clip = u_projection * u_view * vec4(hitPosition, 1.0);

    float ndcDepth = clip.z / clip.w;

    float depth = ndcDepth * 0.5 + 0.5;



    // Write actual sphere depth, not bounding cube depth
    gl_FragDepth = depth;

    gl_FragColor = u_isEdge
        ? shadeEdge(normal, rayDir)
        : shadeSphere(hitPosition, center, rayDir);
}
)POLY_FRAG_SHADER"
