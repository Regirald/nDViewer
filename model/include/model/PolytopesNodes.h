#pragma once
#include "ImplicitFunction.h"
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <QVector3D>
class PolytopeNode : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	// Each node is defined by a position and a radius
	PolytopeNode(const Point& node, float radius)
		: node(node), radius(radius)
	{
		// initialize m_Bounds
		m_boundsMin[0] = std::min(m_boundsMin[0], node[0] - radius);
		m_boundsMin[1] = std::min(m_boundsMin[1], node[1] - radius);
		m_boundsMin[2] = std::min(m_boundsMin[2], node[2] - radius);
		m_boundsMax[0] = std::max(m_boundsMax[0], node[0] + radius);
		m_boundsMax[1] = std::max(m_boundsMax[1], node[1] + radius);
		m_boundsMax[2] = std::max(m_boundsMax[2], node[2] + radius);
	}


	float evaluateSDF(const Point& p) const override
	{
		float dx = p[0] - node[0];
		float dy = p[1] - node[1];
		float dz = p[2] - node[2];
		float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
		return dist - radius;
	}

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_node");

		mutable_u_p = ctx.makeUniform("u_p", "vec3");
		mutable_u_r = ctx.makeUniform("u_radius", "float");

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    vec3 delta = p - " + mutable_u_p + ";");
		ctx.append("    float r = " + mutable_u_r + ";");
		ctx.append("    return length(delta) - r;");
		ctx.append("}");

		return fname;
	}

	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);
		program.setUniformValue(mutable_u_p.c_str(), QVector3D(node[0], node[1], node[2]));
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
					vec3 yellow = vec3(0.75, 0.75, 0.0);
					vec3 color = yellow * (1 - intensity) + white * (intensity);
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

	Point node;
	float radius;

	mutable std::string mutable_u_p;
	mutable std::string mutable_u_r;
};
