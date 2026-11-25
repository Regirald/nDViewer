#pragma once

#include "ImplicitFunction.h"
#include "PolytopeShaderBuilder.h"

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

		m_uniformsNames.resize(2 + m_faceVertices.size() + 1);
		for (size_t j = 0; j < m_faceVertices.size(); ++j)
			m_uniformsNames[2 + j] = "u_face_vertex_" + std::to_string(j);
		m_uniformsNames.back() = "u_face_normal";

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

	const std::string buildSDF() const override
	{
		std::string source = "float " + m_dstFunctionName + "(vec3 p) {\n";
		source += "    vec3 n = normalize(" + m_uniformsNames.back() + ");\n";
		source += "    vec3 a = " + m_uniformsNames[2] + ";\n";
		source += "    float planeDist = dot(p - a, n);\n";
		source += "    vec3 q = p - planeDist * n;\n";
		source += "    float edgeDist2 = 1e30;\n";
		source += "    bool inside = true;\n";

		for (size_t j = 0; j < m_faceVertices.size(); ++j)
		{
			size_t next = (j + 1) % m_faceVertices.size();
			const std::string& aName = m_uniformsNames[2 + j];
			const std::string& bName = m_uniformsNames[2 + next];

			source += "    {\n";
			source += "        vec3 va = " + aName + ";\n";
			source += "        vec3 vb = " + bName + ";\n";
			source += "        vec3 edge = vb - va;\n";
			source += "        vec3 v = q - va;\n";
			source += "        float edgeLen2 = dot(edge, edge);\n";
			source += "        float t = 0.0;\n";
			source += "        if (edgeLen2 > 1e-12)\n";
			source += "            t = clamp(dot(v, edge) / edgeLen2, 0.0, 1.0);\n";
			source += "        vec3 closest = va + t * edge;\n";
			source += "        vec3 diff = q - closest;\n";
			source += "        edgeDist2 = min(edgeDist2, dot(diff, diff));\n";
			source += "        if (dot(cross(edge, v), n) < -1e-6)\n";
			source += "            inside = false;\n";
			source += "    }\n";
		}

		source += "    if (inside)\n";
		source += "        return abs(planeDist);\n";
		source += "    return sqrt(planeDist * planeDist + edgeDist2);\n";
		source += "}\n";
		return source;
	}

	const std::string buildUniforms() const override
	{
		std::string uniforms = ImplicitFunction<3>::buildUniforms();
		for (size_t j = 0; j < m_faceVertices.size(); ++j)
			uniforms += "uniform vec3 " + m_uniformsNames[2 + j] + ";\n";
		uniforms += "uniform vec3 " + m_uniformsNames.back() + ";\n";
		return uniforms;
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
				m_uniformsNames[2 + j].c_str(),
				QVector3D(p[0], p[1], p[2]));
		}

		program.setUniformValue(
			m_uniformsNames.back().c_str(),
			QVector3D(m_faceNormal[0], m_faceNormal[1], m_faceNormal[2]));
	}


	// -------------------------------------------------------------------------
	// Fragment shader (reuses original raymarching logic).
	// -------------------------------------------------------------------------

	const std::string getFragmentShader() const override
	{
		return PolytopeShaderBuilder::buildFragmentShader(buildUniforms(), buildSDF(), PolytopeShaderBuilder::PolytopeShaderStyle::Face);

	}


private:

	// Polygon vertices (in local ordering).
	std::vector<Point> m_faceVertices;

	// Oriented normal.
	Point m_faceNormal;

	// Generated GLSL uniform names
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
