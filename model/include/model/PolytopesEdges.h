#pragma once
#include "ImplicitFunction.h"
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <QVector3D>
class PolytopeEdge : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	// A single cylinder defined by two endpoints and a radius
	PolytopeEdge(const std::pair<Point, Point>& segment, float r)
		: a(segment.first), b(segment.second), radius(r)
	{
		// initialize bounds
		m_boundsMin[0] = std::min(m_boundsMin[0], std::min(a[0], b[0]) - radius);
		m_boundsMin[1] = std::min(m_boundsMin[1], std::min(a[1], b[1]) - radius);
		m_boundsMin[2] = std::min(m_boundsMin[2], std::min(a[2], b[2]) - radius);
		m_boundsMax[0] = std::max(m_boundsMax[0], std::max(a[0], b[0]) + radius);
		m_boundsMax[1] = std::max(m_boundsMax[1], std::max(a[1], b[1]) + radius);
		m_boundsMax[2] = std::max(m_boundsMax[2], std::max(a[2], b[2]) + radius);
	}

	float evaluateSDF(const Point& p) const override
	{
		// distance from point p to segment ab
		float vx = b[0] - a[0];
		float vy = b[1] - a[1];
		float vz = b[2] - a[2];
		float wx = p[0] - a[0];
		float wy = p[1] - a[1];
		float wz = p[2] - a[2];
		float c1 = vx*wx + vy*wy + vz*wz;
		float c2 = vx*vx + vy*vy + vz*vz;
		float t = 0.0f;
		if (c2 > 1e-8f) t = std::max(0.0f, std::min(1.0f, c1 / c2));
		float projx = a[0] + t * vx;
		float projy = a[1] + t * vy;
		float projz = a[2] + t * vz;
		float dist = std::sqrt((p[0]-projx)*(p[0]-projx) + (p[1]-projy)*(p[1]-projy) + (p[2]-projz)*(p[2]-projz));
		return dist - radius;
	}

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_edge");

		mutable_u_p0 = ctx.makeUniform("u_p0", "vec3");
		mutable_u_p1 = ctx.makeUniform("u_p1", "vec3");
		mutable_u_r = ctx.makeUniform("u_radius", "float");

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    vec3 a = " + mutable_u_p0 + ";");
		ctx.append("    vec3 b = " + mutable_u_p1 + ";");
		ctx.append("    vec3 ab = b - a;");
		ctx.append("    vec3 ap = p - a;");
		ctx.append("    float c1 = dot(ab, ap);");
		ctx.append("    float c2 = dot(ab, ab);");
		ctx.append("    float t = 0.0;");
		ctx.append("    if (c2 > 1e-8) t = clamp(c1 / c2, 0.0, 1.0);");
		ctx.append("    vec3 proj = a + t * ab;");
		ctx.append("    float dist = length(p - proj);");
		ctx.append("    return dist - " + mutable_u_r + ";");
		ctx.append("}");

		return fname;
	}


	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);
		program.setUniformValue(mutable_u_p0.c_str(), QVector3D(a[0], a[1], a[2]));
		program.setUniformValue(mutable_u_p1.c_str(), QVector3D(b[0], b[1], b[2]));
		program.setUniformValue(mutable_u_r.c_str(), radius);
	}


	const std::string getFragmentShader() const override
	{
		GLSLContext ctx;

		std::string rootFunc = buildGLSL(ctx);

		std::string sdfEntry =
			"float sampleSDF(vec3 p)\n"
			"{\n"
			"    return " + rootFunc + "(p);\n"
			"}\n";

		const char* part1 = R"(
		#version 330 core

		in vec2 v_uv;
		out vec4 FragColor;

		uniform vec3 u_cameraPos;
		uniform vec3 u_lightDir;
		uniform mat4 u_view;
		uniform mat4 u_invView;
		uniform mat4 u_projection;
		uniform mat4 u_invProjection;

		uniform vec3 u_boundsMin;
		uniform vec3 u_boundsMax;

		uniform float u_maxDistance;
		uniform int u_maxSteps;
		uniform float u_epsilon;

		// -------------------------
		// AABB intersection
		// -------------------------
		bool intersectAABB(vec3 ro, vec3 rd, out float tmin, out float tmax)
		{
			vec3 invDir = 1.0 / rd;
			vec3 t0 = (u_boundsMin - ro) * invDir;
			vec3 t1 = (u_boundsMax - ro) * invDir;

			vec3 tsmaller = min(t0, t1);
			vec3 tbigger  = max(t0, t1);

			tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
			tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

			return tmax > max(tmin, 0.0);
		}
)";

		const char* part2 = R"(

		vec3 estimateNormal(vec3 p)
		{
			float e = 0.002;

			return normalize(vec3(
				sampleSDF(p + vec3(e,0,0)) - sampleSDF(p - vec3(e,0,0)),
				sampleSDF(p + vec3(0,e,0)) - sampleSDF(p - vec3(0,e,0)),
				sampleSDF(p + vec3(0,0,e)) - sampleSDF(p - vec3(0,0,e))
			));
		}

		void main()
		{
			vec2 ndc = v_uv * 2.0 - 1.0;

			vec4 rayClip = vec4(ndc, -1.0, 1.0);
			vec4 rayView = u_invProjection * rayClip;
			rayView = vec4(rayView.xyz, 0.0);

			vec3 rayDir = normalize((u_invView * rayView).xyz);
			vec3 rayOrigin = u_cameraPos;

			float tmin, tmax;
			if (!intersectAABB(rayOrigin, rayDir, tmin, tmax))
			{
				discard;
			}

			float t = max(tmin, 0.0);

			for (int i = 0; i < u_maxSteps; ++i)
			{
				vec3 p = rayOrigin + t * rayDir;
				float d = sampleSDF(p);

				if (abs(d) < u_epsilon)
				{    
					vec3 n = estimateNormal(p);           
					vec3 rayDirReflected = reflect(rayDir, n);
					float intensity = dot(u_lightDir, rayDirReflected);
					vec3 white = vec3(0.85);
					vec3 grey= vec3(0.75);
					vec3 color = grey * (1 - intensity) + white * (intensity);
					float alpha = 1.0;
					FragColor = vec4(color, alpha);

					// ---- Depth write ----
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
)";

		return
			std::string(part1)
			+ ctx.uniforms
			+ ctx.code
			+ sdfEntry
			+ std::string(part2);
	}

	Point a;
	Point b;
	float radius;

	mutable std::string mutable_u_p0;
	mutable std::string mutable_u_p1;
	mutable std::string mutable_u_r;
};
