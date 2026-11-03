#pragma once
#include "ImplicitFunction.h"
#include <vector>
#include <string>
#include <QVector3D>

class SpheresCollection : public ImplicitFunction<3> {
public:
	using Point = ImplicitFunction<3>::Point;

	SpheresCollection(const std::vector<Point>& centers, const std::vector<float>& radii)
		: centers(centers), radii(radii)
	{
	// initialize bound min max
		for (int i =0; i < centers.size(); i++)
		{
			const auto& center = centers[i];

			m_boundsMin[0] = std::min(m_boundsMin[0], center[0]) - radii[i];
			m_boundsMin[1] = std::min(m_boundsMin[1], center[1]) - radii[i];
			m_boundsMin[2] = std::min(m_boundsMin[2], center[2]) - radii[i];
			m_boundsMax[0] = std::max(m_boundsMax[0], center[0]) + radii[i];
			m_boundsMax[1] = std::max(m_boundsMax[1], center[1]) + radii[i];
			m_boundsMax[2] = std::max(m_boundsMax[2], center[2]) + radii[i];
		}
	}

	float evaluateSDF(const Point& p) const override
	{
		float d = std::numeric_limits<float>::infinity();
		for (size_t i = 0; i < centers.size(); ++i)
		{
			float cur = std::sqrt((p[0] - centers[i][0])*(p[0] - centers[i][0]) +
								  (p[1] - centers[i][1])*(p[1] - centers[i][1]) +
								  (p[2] - centers[i][2])*(p[2] - centers[i][2])) - radii[i];
			d = std::min(d, cur);
		}
		return d;
	}

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_spheres");

		// create per-sphere uniforms and record their names
		mutable_u_centers.clear();
		mutable_u_radii.clear();
		for (size_t i = 0; i < centers.size(); ++i)
		{
			mutable_u_centers.push_back(ctx.makeUniformUnique("u_center", "vec3"));
			mutable_u_radii.push_back(ctx.makeUniformUnique("u_radius", "float"));
		}

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    float d = 1e9;");
		for (size_t i = 0; i < centers.size(); ++i)
		{
			ctx.append("    d = min(d, length(p - " + mutable_u_centers[i] + ") - " + mutable_u_radii[i] + ");");
		}
		ctx.append("    return d;");
		ctx.append("}");

		return fname;
	}

	const std::string getGLSL_parameters() const override
	{
		return std::string();
	}

	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);

		// set individual uniforms
		for (size_t i = 0; i < centers.size(); ++i)
		{
			const std::string& nameC = mutable_u_centers[i];
			const std::string& nameR = mutable_u_radii[i];
			program.setUniformValue(nameC.c_str(), QVector3D(centers[i][0], centers[i][1], centers[i][2]));
			program.setUniformValue(nameR.c_str(), radii[i]);
		}
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


	std::vector<Point> centers;
	std::vector<float> radii;

	// mutable only so setGLUniforms can use them (buildGLSL is const)
	mutable std::vector<std::string> mutable_u_centers;
	mutable std::vector<std::string> mutable_u_radii;
};
