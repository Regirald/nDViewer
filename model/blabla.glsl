
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

int u_primitiveToutchedIndex = 0;


#extension GL_ARB_shader_storage_buffer_object : require
layout(std430, binding = 0) buffer VerticeSetData { vec4 u_vertices[]; };
uniform int u_bvhVertexCount;
uniform float u_radius;


float sampleVertexBruteForce(vec3 p) {
	float bestDistance = 1e10;
	for (int vertexIndex = 0; vertexIndex < u_bvhVertexCount; ++vertexIndex)
	{
		vec4 vertex = u_vertices[vertexIndex];
		vec3 vertex3d = vertex.xyz / (1 - 0.3 * vertex.w);
		float newDistance = length(p - vertex3d) - u_radius;
		if(newDistance < bestDistance)
		{
			u_primitiveToutchedIndex = vertexIndex;
			bestDistance = newDistance;
		}
	}
    return bestDistance;
}
float SDFonPrimitive(vec3 p) {
	float bestDistance = 1e10;
	int vertexIndex = u_primitiveToutchedIndex;
	vec4 vertex = u_vertices[vertexIndex];
	vec3 vertex3d = vertex.xyz / (1 - 0.3 * vertex.w);
	bestDistance = min(bestDistance, length(p - vertex3d) - u_radius);
    return bestDistance;
}


float sampleSDF(vec3 p)
{
    return sampleVertexBruteForce(p);
}



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
		SDFonPrimitive(p + vec3(e, 0, 0)) - SDFonPrimitive(p - vec3(e, 0, 0)),
		SDFonPrimitive(p + vec3(0, e, 0)) - SDFonPrimitive(p - vec3(0, e, 0)),
		SDFonPrimitive(p + vec3(0, 0, e)) - SDFonPrimitive(p - vec3(0, 0, e))
	));
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
			gl_FragColor = shadeNode(p, rayDir);

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
