
#version 330 core

in vec2 v_uv;
out vec4 FragColor;

uniform vec3 u_cameraPos;
uniform vec3 u_lightDir;
uniform vec3 u_face_normal;
uniform mat4 u_view;
uniform mat4 u_invView;
uniform mat4 u_projection;
uniform mat4 u_invProjection;

uniform vec3 u_boundsMin;
uniform vec3 u_boundsMax;

uniform float u_maxDistance;
uniform int u_maxSteps;
uniform float u_epsilon;

#define POLYTOPE_VERTEX_BVH

#ifdef POLYTOPE_VERTEX_BVH
uniform samplerBuffer u_bvhData;
uniform int u_bvhNodeCount;
uniform int u_bvhVertexOffset;
uniform int u_bvhPrimitiveOffset;
#endif
float bvhBoundsDistance(vec3 p, int nodeIndex) {
    vec3 boundsMin = texelFetch(u_bvhData, nodeIndex * 3).xyz;
    vec3 boundsMax = texelFetch(u_bvhData, nodeIndex * 3 + 1).xyz;
    vec3 outside = max(max(boundsMin - p, p - boundsMax), vec3(0.0));
    return length(outside) - texelFetch(u_bvhData, nodeIndex * 3).w;
}
float sampleVertexBVH(vec3 p) {
    if (u_bvhNodeCount == 0) return 1e20;
    int stack[64];
    int stackSize = 1;
    int visited = 0;
    float bestDistance = 1e20;
    stack[0] = 0;
    while (stackSize > 0 && visited < u_bvhNodeCount) {
        int nodeIndex = stack[--stackSize];
        ++visited;
        if (bvhBoundsDistance(p, nodeIndex) > bestDistance) continue;
        vec4 metadata = texelFetch(u_bvhData, nodeIndex * 3 + 2);
        int primitiveCount = int(metadata.w);
        if (primitiveCount > 0) {
            int firstPrimitive = int(metadata.z);
            for (int i = 0; i < primitiveCount; ++i) {
                int vertexIndex = int(texelFetch(u_bvhData, u_bvhPrimitiveOffset + firstPrimitive + i).x);
                vec4 vertex = texelFetch(u_bvhData, u_bvhVertexOffset + vertexIndex);
                bestDistance = min(bestDistance, length(p - vertex.xyz) - vertex.w);
            }
        } else {
            int leftChild = int(metadata.x);
            int rightChild = int(metadata.y);
            if (leftChild >= 0 && bvhBoundsDistance(p, leftChild) <= bestDistance && stackSize < 64) stack[stackSize++] = leftChild;
            if (rightChild >= 0 && bvhBoundsDistance(p, rightChild) <= bestDistance && stackSize < 64) stack[stackSize++] = rightChild;
        }
    }
    return bestDistance;
}

#ifdef POLYTOPE_VERTEX_BVH
float sampleSDF(vec3 p)
{
	return sampleVertexBVH(p);
}
#else
float sampleSDF(vec3 p)
{
    return sampleVertexBVH(p);
}

#endif

bool intersectAABB(vec3 ro, vec3 rd, out float tmin, out float tmax)
{
	vec3 invDir = 1.0 / rd;
	vec3 t0 = (u_boundsMin - ro) * invDir;
	vec3 t1 = (u_boundsMax - ro) * invDir;

	vec3 tsmaller = min(t0, t1);
	vec3 tbigger = max(t0, t1);

	tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
	tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

	return tmax > max(tmin, 0.0);
}

vec3 estimateNormal(vec3 p)
{
	float e = 0.002;

	return normalize(vec3(
		sampleSDF(p + vec3(e, 0, 0)) - sampleSDF(p - vec3(e, 0, 0)),
		sampleSDF(p + vec3(0, e, 0)) - sampleSDF(p - vec3(0, e, 0)),
		sampleSDF(p + vec3(0, 0, e)) - sampleSDF(p - vec3(0, 0, e))
	));
}

vec4 shadeFace(vec3 p, vec3 rayDir)
{
	vec3 n = normalize(u_face_normal);
	vec3 v = normalize(-rayDir);
	vec3 l = normalize(u_lightDir);
	vec3 r = reflect(-v, n);
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

vec4 shadeEdge(vec3 p, vec3 rayDir)
{
	vec3 n = estimateNormal(p);
	vec3 rayDirReflected = reflect(rayDir, n);
	float intensity = dot(u_lightDir, rayDirReflected);
	vec3 white = vec3(0.85);
	vec3 grey = vec3(0.75);
	vec3 color = grey * (1.0 - intensity) + white * intensity;
	return vec4(color, 1.0);
}

vec4 shadeNode(vec3 p, vec3 rayDir)
{
	vec3 n = estimateNormal(p);
	vec3 rayDirReflected = reflect(rayDir, n);
	float intensity = dot(u_lightDir, rayDirReflected);
	vec3 white = vec3(0.85);
	vec3 yellow = vec3(0.75, 0.75, 0.0);
	vec3 color = yellow * (1.0 - intensity) + white * intensity;
	return vec4(color, 1.0);
}

void main()
{
	vec2 ndc = v_uv * 2.0 - 1.0;
	vec4 rayClip = vec4(ndc, -1.0, 1.0);
	vec4 rayView = u_invProjection * rayClip;
	rayView /= rayView.w;
	vec3 rayDir = normalize((u_invView * vec4(rayView.xyz, 0.0)).xyz);
	vec3 rayOrigin = u_cameraPos;

	float tmin, tmax;
	if (!intersectAABB(rayOrigin, rayDir, tmin, tmax))
		discard;

	float t = max(tmin, 0.0);
	for (int i = 0; i < u_maxSteps; ++i)
	{
		vec3 p = rayOrigin + t * rayDir;
		float d = sampleSDF(p);

		if (abs(d) < u_epsilon)
		{
			FragColor = shadeNode(p, rayDir);

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
