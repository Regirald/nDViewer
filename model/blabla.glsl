
#version 330 core

#define POLYTOPE_DIMENSION 4

in vec2 v_uv;
out vec4 FragColor;

uniform vec3 u_cameraPos;
uniform vec3 u_lightDir;
uniform vec3 u_face_normal;

uniform mat4 u_view;
uniform mat4 u_invView;
uniform mat4 u_projection;
uniform mat4 u_invProjection;

uniform vec4 u_boundsMin;
uniform vec4 u_boundsMax;

uniform float u_maxDistance;
uniform int u_maxSteps;
uniform float u_epsilon;

uniform samplerBuffer u_bvhData;uniform int u_bvhNodeCount;uniform int u_bvhVertexOffset;uniform int u_bvhPrimitiveOffset;

float bvhBoundsDistance(vec4 p, int nodeOffset) {
    vec4 boundsMin;
    vec4 boundsMax;
    for (int i = 0; i < 4; ++i) {
        boundsMin[i] = texelFetch(u_bvhData, nodeOffset + i).r;
        boundsMax[i] = texelFetch(u_bvhData, nodeOffset + 4 + i).r;
    }
    vec4 outside = max(max(boundsMin - p, p - boundsMax), vec4(0.0));
    float nodeRadius = texelFetch(u_bvhData, nodeOffset + 12).r;
    return length(outside) - nodeRadius;
}
float sampleVertexBVH(vec4 p) {
    if (u_bvhNodeCount == 0) return 1e20;
    int stack[64];
    int stackSize = 1;
    int visited = 0;
    float bestDistance = 1e20;
    stack[0] = 0;
    while (stackSize > 0 && visited < u_bvhNodeCount) {
        int nodeIndex = stack[--stackSize];
        ++visited;
        int nodeOffset = nodeIndex * 13;
        if (bvhBoundsDistance(p, nodeOffset) > bestDistance) continue;
        int metadataOffset = nodeOffset + 8;
        int leftChild = int(texelFetch(u_bvhData, metadataOffset).r);
        int rightChild = int(texelFetch(u_bvhData, metadataOffset + 1).r);
        int firstPrimitive = int(texelFetch(u_bvhData, metadataOffset + 2).r);
        int primitiveCount = int(texelFetch(u_bvhData, metadataOffset + 3).r);
        if (primitiveCount > 0) {
            for (int i = 0; i < primitiveCount; ++i) {
                int primitiveOffset = u_bvhPrimitiveOffset + firstPrimitive + i;
                int vertexIndex = int(texelFetch(u_bvhData, primitiveOffset).r);
                int vertexOffset = u_bvhVertexOffset + vertexIndex * 5;
                vec4 vertex;
                for (int i = 0; i < 4; ++i)
                    vertex[i] = texelFetch(u_bvhData, vertexOffset + i).r;
                float vertexRadius = texelFetch(u_bvhData, vertexOffset + 4).r;
                bestDistance = min(bestDistance, length(p - vertex) - vertexRadius);
            }
        } else {
            int leftOffset = leftChild * 13;
            int rightOffset = rightChild * 13;
            if (leftChild >= 0 && bvhBoundsDistance(p, leftOffset) <= bestDistance && stackSize < 64)
                stack[stackSize++] = leftChild;
            if (rightChild >= 0 && bvhBoundsDistance(p, rightOffset) <= bestDistance && stackSize < 64)
                stack[stackSize++] = rightChild;
        }
    }
    return bestDistance;
}


float sampleSDF(vec4 p)
{
    return sampleVertexBVH(p);
}



// -----------------------------------------------------------------------------
// Convert a 3D camera vector into the geometry's dimensional space.
// -----------------------------------------------------------------------------

vec4 cameraToGeometry(vec3 v)
{
#if POLYTOPE_DIMENSION == 2
    return v.xy;
#elif POLYTOPE_DIMENSION == 3
    return v;
#elif POLYTOPE_DIMENSION == 4
    return vec4(v, 0.0);
#endif
}


vec3 geometryToWorld(vec4 p)
{
#if POLYTOPE_DIMENSION == 2
    return vec3(p, 0.0);
#elif POLYTOPE_DIMENSION == 3
    return p;
#elif POLYTOPE_DIMENSION == 4
    return p.xyz; // Valid for a fixed-W 3D slice
#endif
}

// -----------------------------------------------------------------------------
// AABB intersection
// -----------------------------------------------------------------------------

bool intersectAABB(
    vec4 ro,
    vec4 rd,
    out float tmin,
    out float tmax)
{
    tmin = -1e20;
    tmax =  1e20;

    for (int i = 0; i < POLYTOPE_DIMENSION; ++i)
    {
        float invDir = 1.0 / rd[i];

        float t0 = (u_boundsMin[i] - ro[i]) * invDir;
        float t1 = (u_boundsMax[i] - ro[i]) * invDir;

        tmin = max(tmin, min(t0, t1));
        tmax = min(tmax, max(t0, t1));
    }

    return tmax > max(tmin, 0.0);
}


// -----------------------------------------------------------------------------
// Normal estimation
// -----------------------------------------------------------------------------

vec4 estimateNormal(vec4 p)
{
    float e = 0.002;

    vec4 gradient = vec4(0.0);

    for (int i = 0; i < POLYTOPE_DIMENSION; ++i)
    {
        vec4 offset = vec4(0.0);
        offset[i] = e;

        gradient[i] =
            sampleSDF(p + offset) -
            sampleSDF(p - offset);
    }

    return normalize(gradient);
}


// -----------------------------------------------------------------------------
// Face shading
// -----------------------------------------------------------------------------

vec4 shadeFace(
    vec4 p,
    vec4 rayDir)
{
    vec4 n = normalize(cameraToGeometry(u_face_normal));
    vec4 v = normalize(-rayDir);
    vec4 l = normalize(cameraToGeometry(u_lightDir));

    vec4 r = reflect(-v, n);

    float facingLight = max(dot(r, l), 0.0);

    float broad = pow(facingLight, 8.0);
    float sharp = pow(facingLight, 64.0);

    vec3 baseColor = vec3(0.025, 0.08, 0.50);
    vec3 reflectedColor = vec3(0.55, 0.72, 1.0);

    vec3 color = baseColor;

    color += reflectedColor * broad * 0.45;
    color += vec3(0.9, 0.95, 1.0) * sharp * 0.5;

    return vec4(color, 0.5);
}


// -----------------------------------------------------------------------------
// Edge shading
// -----------------------------------------------------------------------------

vec4 shadeEdge(
    vec4 p,
    vec4 rayDir)
{
    vec4 n = estimateNormal(p);

    vec4 rayDirReflected = reflect(rayDir, n);

    float intensity = dot(cameraToGeometry(u_lightDir), rayDirReflected);

    vec3 white = vec3(0.85);
    vec3 grey = vec3(0.75);

    vec3 color = grey * (1.0 - intensity) + white * intensity;

    return vec4(color, 1.0);
}


// -----------------------------------------------------------------------------
// Node shading
// -----------------------------------------------------------------------------

vec4 shadeNode(
    vec4 p,
    vec4 rayDir)
{
    vec4 n = estimateNormal(p);

    vec4 rayDirReflected = reflect(rayDir, n);

    float intensity = dot(cameraToGeometry(u_lightDir), rayDirReflected);

    vec3 white = vec3(0.85);
    vec3 yellow = vec3(0.75, 0.75, 0.0);

    vec3 color = yellow * (1.0 - intensity) + white * intensity;

    return vec4(color, 1.0);
}


// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void main()
{
    // 2D screen coordinates.

    vec2 ndc = v_uv * 2.0 - 1.0;

    // Generate the camera ray in ordinary 3D space.

    vec4 rayClip = vec4(ndc, -1.0, 1.0);

    vec4 rayView = u_invProjection * rayClip;
    rayView /= rayView.w;

    vec3 cameraRayDir = normalize((u_invView * vec4(rayView.xyz, 0.0)).xyz
    );

    // Embed camera position and direction into geometry space.

    vec4 rayOrigin = cameraToGeometry(u_cameraPos);

    vec4 rayDir = normalize(cameraToGeometry(cameraRayDir)
    );

    // AABB intersection.

    float tmin, tmax;

    if (!intersectAABB(rayOrigin, rayDir, tmin, tmax))
        discard;

    // Sphere tracing.

    float t = max(tmin, 0.0);

    for (int i = 0; i < u_maxSteps; ++i)
    {
        vec4 p = rayOrigin + t * rayDir;

        float d = sampleSDF(p);

        if (abs(d) < u_epsilon)
        {
            FragColor = shadeNode(p, rayDir);

            // Depth projection remains entirely 3D.

            vec3 hitPosition = geometryToWorld(p);
            vec4 clip = u_projection * u_view * vec4(hitPosition, 1.0);

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
