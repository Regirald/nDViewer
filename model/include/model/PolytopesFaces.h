#pragma once

#include "ImplicitFunction.h"

#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <algorithm>

#include <QVector3D>


// Single-face polytope SDF. Replaces the previous multi-face implementation
// with a class that represents one polygonal face (convexity assumed for
// inside/outside checks).
class PolytopeFace : public ImplicitFunction<3>
{
public:

	using Point = ImplicitFunction<3>::Point;

	// Construct from a single face given as a list of vertex positions.
	PolytopeFace(const std::vector<Point>& faceVertices)
		: m_faceVertices(faceVertices)
	{
		// Compute bounds from the face vertices.
		m_boundsMin = QVector3D(
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max());

		m_boundsMax = QVector3D(
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest());

		for (const auto& p : m_faceVertices)
		{
			m_boundsMin[0] = std::min(m_boundsMin[0], p[0]);
			m_boundsMin[1] = std::min(m_boundsMin[1], p[1]);
			m_boundsMin[2] = std::min(m_boundsMin[2], p[2]);

			m_boundsMax[0] = std::max(m_boundsMax[0], p[0]);
			m_boundsMax[1] = std::max(m_boundsMax[1], p[1]);
			m_boundsMax[2] = std::max(m_boundsMax[2], p[2]);
		}

		const float padding = 10.f;  //TODO: remove, secure bounds for now
		m_boundsMin -= QVector3D(padding, padding, padding);
		m_boundsMax += QVector3D(padding, padding, padding);

		// Reorder polygon vertices to ensure consistent winding.
		reorderFaceConvexHull();

		// Compute face normal (or default if degenerate).
		if (m_faceVertices.size() < 3)
		{
			m_faceNormal = Point{ 0.0f, 0.0f, 1.0f };
		}
		else
		{
			const Point& a = m_faceVertices[0];
			const Point& b = m_faceVertices[1];
			const Point& c = m_faceVertices[2];

			Point ab = b - a;
			Point ac = c - a;

			Point n = cross3D(ab, ac);

			float nlen2 = dot(n, n);
			if (nlen2 < 1e-12f)
				m_faceNormal = Point{ 0.0f, 0.0f, 1.0f };
			else
				m_faceNormal = normalize(n);
		}
	}


	// -------------------------------------------------------------------------
	// CPU SDF for a single polygonal face.
	// -------------------------------------------------------------------------

	float evaluateSDF(const Point& p) const override
	{
		if (m_faceVertices.size() < 3)
			return std::numeric_limits<float>::infinity();

		const Point& a = m_faceVertices[0];
		const Point& n = m_faceNormal;

		// Signed distance to infinite plane.
		float planeDist = dot(p - a, n);

		// Project p onto the face plane.
		Point q = p - planeDist * n;

		// Distance to polygon boundary.
		float edgeDist2 = std::numeric_limits<float>::infinity();
		bool inside = true;

		for (size_t j = 0; j < m_faceVertices.size(); ++j)
		{
			const Point& v0 = m_faceVertices[j];
			const Point& v1 = m_faceVertices[(j + 1) % m_faceVertices.size()];

			Point edge = v1 - v0;
			Point v = q - v0;

			float edgeLen2 = dot(edge, edge);
			float t = 0.0f;
			if (edgeLen2 > 1e-12f)
			{
				t = dot(v, edge) / edgeLen2;
				t = std::clamp(t, 0.0f, 1.0f);
			}

			Point closest = v0 + t * edge;
			Point distVec = q - closest;
			float dist2 = dot(distVec, distVec);
			edgeDist2 = std::min(edgeDist2, dist2);

			// Convex polygon inside test.
			Point cross = cross3D(edge, v);

			float side = dot(cross, n);
			if (side < -1e-6f)
				inside = false;
		}

		if (inside)
		{
			return std::abs(planeDist);
		}
		else
		{
			return std::sqrt(planeDist * planeDist + edgeDist2);
		}
	}


	// -------------------------------------------------------------------------
	// GLSL generation for this face.
	// -------------------------------------------------------------------------

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_face");

		// One uniform per vertex.
		mutable_u_vertices.clear();
		mutable_u_normal.clear();

		mutable_u_vertices.resize(m_faceVertices.size());
		for (size_t j = 0; j < m_faceVertices.size(); ++j)
			mutable_u_vertices[j] = ctx.makeUniform("u_face_vertex", "vec3");

		mutable_u_normal = ctx.makeUniform("u_face_normal", "vec3");

		ctx.append("float " + fname + "(vec3 p) {");
		ctx.append("    vec3 n = normalize(" + mutable_u_normal + ");");
		ctx.append("    vec3 a = " + mutable_u_vertices[0] + ";");
		ctx.append("    float planeDist = dot(p - a, n);");
		ctx.append("    vec3 q = p - planeDist * n;");
		ctx.append("    float edgeDist2 = 1e30;");
		ctx.append("    bool inside = true;");

		for (size_t j = 0; j < m_faceVertices.size(); ++j)
		{
			size_t next = (j + 1) % m_faceVertices.size();
			std::string aName = mutable_u_vertices[j];
			std::string bName = mutable_u_vertices[next];

			ctx.append("    {");
			ctx.append("        vec3 va = " + aName + ";");
			ctx.append("        vec3 vb = " + bName + ";");
			ctx.append("        vec3 edge = vb - va;");
			ctx.append("        vec3 v = q - va;");
			ctx.append("        float edgeLen2 = dot(edge, edge);");
			ctx.append("        float t = 0.0;");
			ctx.append("        if (edgeLen2 > 1e-12)");
			ctx.append("            t = clamp(dot(v, edge) / edgeLen2, 0.0, 1.0);");
			ctx.append("        vec3 closest = va + t * edge;");
			ctx.append("        vec3 diff = q - closest;");
			ctx.append("        edgeDist2 = min(edgeDist2, dot(diff, diff));");
			ctx.append("        if (dot(cross(edge, v), n) < -1e-6)");
			ctx.append("            inside = false;");
			ctx.append("    }");
		}

		ctx.append("    if (inside)");
		ctx.append("        return abs(planeDist);");
		ctx.append("    return sqrt(planeDist * planeDist + edgeDist2);");
		ctx.append("}");

		return fname;
	}


	const std::string getGLSL_parameters() const override
	{
		return std::string();
	}


	// -------------------------------------------------------------------------
	// Uniform upload
	// -------------------------------------------------------------------------

	void setGLUniforms(QOpenGLShaderProgram& program) const override
	{
		ImplicitFunction<3>::setGLUniforms(program);

		for (size_t j = 0; j < m_faceVertices.size(); ++j)
		{
			const Point& p = m_faceVertices[j];
			program.setUniformValue(
				mutable_u_vertices[j].c_str(),
				QVector3D(p[0], p[1], p[2]));
		}

		program.setUniformValue(
			mutable_u_normal.c_str(),
			QVector3D(m_faceNormal[0], m_faceNormal[1], m_faceNormal[2]));
	}


	// -------------------------------------------------------------------------
	// Fragment shader (reuses original raymarching logic).
	// -------------------------------------------------------------------------

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

		bool intersectAABB(
			vec3 ro,
			vec3 rd,
			out float tmin,
			out float tmax)
		{
			vec3 invDir = 1.0 / rd;

			vec3 t0 = (u_boundsMin - ro) * invDir;
			vec3 t1 = (u_boundsMax - ro) * invDir;

			vec3 tsmaller = min(t0, t1);
			vec3 tbigger  = max(t0, t1);

			tmin = max(
				max(tsmaller.x, tsmaller.y),
				tsmaller.z);

			tmax = min(
				min(tbigger.x, tbigger.y),
				tbigger.z);

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

			vec4 rayClip =
				vec4(ndc, -1.0, 1.0);

			vec4 rayView =
				u_invProjection * rayClip;

			rayView /= rayView.w;

			vec3 rayDir =
				normalize(
					(u_invView *
					 vec4(rayView.xyz, 0.0)).xyz);

			vec3 rayOrigin =
				u_cameraPos;


			float tmin;
			float tmax;

			if (!intersectAABB(
					rayOrigin,
					rayDir,
					tmin,
					tmax))
			{
				discard;
			}


			float t =
				max(tmin, 0.0);


			for (int i = 0;
				 i < u_maxSteps;
				 ++i)
			{
				vec3 p =
					rayOrigin + t * rayDir;

				float d =
					sampleSDF(p);


				if (d < u_epsilon)
				{
					vec3 n = estimateNormal(p);           
					vec3 rayDirReflected = reflect(rayDir, n);
					float intensity = dot(u_lightDir, rayDirReflected);
					vec3 white = vec3(0.85);
					vec3 blue = vec3(0.0, 0.0, 0.75);
					vec3 color = blue * (1 - intensity) + white * (intensity);
					float alpha = 0.5;
					FragColor =  vec4(color, alpha);


					// -------------------------
					// Depth write
					// -------------------------

					vec4 clip =
						u_projection *
						u_view *
						vec4(p, 1.0);

					float ndcDepth =
						clip.z / clip.w;

					gl_FragDepth =
						ndcDepth * 0.5 + 0.5;

					return;
				}


				// The distance is unsigned, so this is
				// intentionally NOT simply "t += d".
				t += max(d * 0.7, 0.001);


				if (t > tmax ||
					t > u_maxDistance)
				{
					break;
				}
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


private:

	// Polygon vertices (in local ordering).
	std::vector<Point> m_faceVertices;

	// Oriented normal.
	Point m_faceNormal;

	// Bounds for the face vertices (AABB).
	QVector3D m_boundsMin;
	QVector3D m_boundsMax;

	// Generated GLSL uniform names
	mutable std::vector<std::string> mutable_u_vertices;
	mutable std::string mutable_u_normal;

	// Reorder a single face's vertices to follow a planar angular ordering
	// around the face centroid.
	void reorderFaceConvexHull()
	{
		size_t m = m_faceVertices.size();
		if (m < 3)
			return;

		// Compute centroid
		Point centroid = { 0.0f, 0.0f, 0.0f };
		for (const auto& p : m_faceVertices)
			centroid += p;
		centroid /= static_cast<float>(m);

		// Estimate face normal by summing cross products.
		Point n = { 0.0f, 0.0f, 0.0f };
		for (size_t i = 0; i < m; ++i)
		{
			const Point& a = m_faceVertices[i];
			const Point& b = m_faceVertices[(i + 1) % m];
			n += cross3D(a, b);
		}

		float nlen2 = dot(n, n);
		if (nlen2 < 1e-12f)
			n = { 0.0f, 0.0f, 1.0f };
		else
			n = normalize(n);

		// Build local 2D frame (u,v) in the face plane.
		Point u = m_faceVertices[0] - centroid;
		if (dot(u, u) < 1e-12f)
		{
			if (std::abs(n[0]) > std::abs(n[1]))
				u = Point{ -n[2], 0.0f, n[0] };
			else
				u = Point{ 0.0f, n[2], -n[1] };
		}
		u = normalize(u);

		Point v = cross3D(n, u);

		// Angle sort around centroid
		std::vector<std::pair<float, Point>> angs;
		angs.reserve(m);
		for (size_t i = 0; i < m; ++i)
		{
			Point r = m_faceVertices[i] - centroid;
			float x = dot(u, r);
			float y = dot(v, r);
			float a = std::atan2(y, x);
			angs.emplace_back(a, m_faceVertices[i]);
		}

		std::sort(angs.begin(), angs.end(),
			[](const std::pair<float, Point>& A, const std::pair<float, Point>& B)
			{
				return A.first < B.first;
			});

		for (size_t i = 0; i < m; ++i)
			m_faceVertices[i] = angs[i].second;
	}
};
