#pragma once

#include "ImplicitFunction.h"

#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <algorithm>

#include <QVector3D>


class PolytopesFaces : public ImplicitFunction<3>
{
public:

	using Point = ImplicitFunction<3>::Point;

	using Faces = std::vector<std::vector<int>>;

	// -------------------------------------------------------------------------
	// Constructor
	// -------------------------------------------------------------------------

	PolytopesFaces(
		const Faces& faces,
		const std::vector<Point>& vertices)
		: faces(faces)
	{
		// -------------------------------------------------------------
		// Compute bounds from ALL vertices.
		// -------------------------------------------------------------

		m_boundsMin = QVector3D(
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max());

		m_boundsMax = QVector3D(
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest());

		for (const auto& face : faces)
		{
			for (int index : face)
			{
				const Point& p = vertices[index];

				m_boundsMin[0] = std::min(m_boundsMin[0], p[0]);
				m_boundsMin[1] = std::min(m_boundsMin[1], p[1]);
				m_boundsMin[2] = std::min(m_boundsMin[2], p[2]);

				m_boundsMax[0] = std::max(m_boundsMax[0], p[0]);
				m_boundsMax[1] = std::max(m_boundsMax[1], p[1]);
				m_boundsMax[2] = std::max(m_boundsMax[2], p[2]);
			}
		}

		// Small padding for numerical robustness.
		const float padding = 0.001f;

		m_boundsMin -= QVector3D(padding, padding, padding);
		m_boundsMax += QVector3D(padding, padding, padding);


		// -------------------------------------------------------------
		// Store actual vertices of each face.
		// -------------------------------------------------------------

		m_faceVertices.reserve(faces.size());

		for (const auto& face : faces)
		{
			std::vector<Point> polygon;
			polygon.reserve(face.size());

			for (int index : face)
				polygon.push_back(vertices[index]);

			m_faceVertices.push_back(std::move(polygon));
		}


		// -------------------------------------------------------------
		// Compute one normal per face.
		//
		// This assumes the vertices of each face are consistently
		// wound.
		// -------------------------------------------------------------

		m_faceNormals.reserve(m_faceVertices.size());

		for (const auto& face : m_faceVertices)
		{
			if (face.size() < 3)
			{
				m_faceNormals.push_back(Point{ 0.0f, 0.0f, 1.0f });
				continue;
			}

			const Point& a = face[0];
			const Point& b = face[1];
			const Point& c = face[2];

			Point ab = b - a;

			Point ac = c - a;

			Point n = {
				ab[1] * ac[2] - ab[2] * ac[1],
				ab[2] * ac[0] - ab[0] * ac[2],
				ab[0] * ac[1] - ab[1] * ac[0]
			};

			n = normalize(n);

			m_faceNormals.push_back(n);
		}
	}


	// -------------------------------------------------------------------------
	// CPU SDF
	// -------------------------------------------------------------------------

	float evaluateSDF(const Point& p) const override
	{
		float d = std::numeric_limits<float>::infinity();

		for (size_t i = 0; i < m_faceVertices.size(); ++i)
		{
			const auto& face = m_faceVertices[i];

			if (face.size() < 3)
				continue;

			const Point& a = face[0];
			const Point& n = m_faceNormals[i];

			// ---------------------------------------------------------
			// Signed distance to infinite plane.
			// ---------------------------------------------------------

			float planeDist =
				(p[0] - a[0]) * n[0] +
				(p[1] - a[1]) * n[1] +
				(p[2] - a[2]) * n[2];

			// ---------------------------------------------------------
			// Project p onto the face plane.
			// ---------------------------------------------------------

			Point q = {
				p[0] - planeDist * n[0],
				p[1] - planeDist * n[1],
				p[2] - planeDist * n[2]
			};

			// ---------------------------------------------------------
			// Distance to polygon boundary.
			// ---------------------------------------------------------

			float edgeDist2 =
				std::numeric_limits<float>::infinity();

			bool inside = true;

			for (size_t j = 0; j < face.size(); ++j)
			{
				const Point& v0 = face[j];
				const Point& v1 = face[(j + 1) % face.size()];

				Point edge = v1 - v0;

				Point v = q - v0;

				float edgeLen2 = dot(edge, edge);

				float t = 0.0f;

				if (edgeLen2 > 1e-12f)
				{
					t = dot(v, edge) / edgeLen2;

					t = std::clamp(t, 0.0f, 1.0f);
				}

				Point closest =  v0 + t * edge;

				Point distVec = q - closest;
				float dist2 = dot(distVec, distVec);

				edgeDist2 = std::min(edgeDist2, dist2);


				// -----------------------------------------------------
				// Convex polygon inside test.
				// -----------------------------------------------------

				Point cross = {
					edge[1] * v[2] - edge[2] * v[1],
					edge[2] * v[0] - edge[0] * v[2],
					edge[0] * v[1] - edge[1] * v[0]
				};

				float side = dot(cross, n);

				if (side < -1e-6f)
					inside = false;
			}

			float faceDist;

			if (inside)
			{
				// Projection is inside polygon.
				// Distance is simply distance to the plane.
				faceDist = std::abs(planeDist);
			}
			else
			{
				// Projection is outside polygon.
				// Exact Euclidean distance to polygon.
				faceDist = std::sqrt(
					planeDist * planeDist +
					edgeDist2);
			}

			d = std::min(d, faceDist);
		}

		return d;
	}


	// -------------------------------------------------------------------------
	// GLSL generation
	// -------------------------------------------------------------------------

	std::string buildGLSL(GLSLContext& ctx) const override
	{
		std::string fname = ctx.makeName("sdf_faces");

		mutable_u_vertices.clear();
		mutable_u_normal.clear();

		mutable_u_vertices.resize(m_faceVertices.size());
		mutable_u_normal.resize(m_faceVertices.size());

		for (size_t i = 0; i < m_faceVertices.size(); ++i)
		{
			const auto& face = m_faceVertices[i];

			// ---------------------------------------------------------
			// One uniform for each vertex.
			// ---------------------------------------------------------

			for (size_t j = 0; j < face.size(); ++j)
			{
				mutable_u_vertices[i].push_back(
					ctx.makeUniform("u_face_vertex", "vec3"));
			}

			// ---------------------------------------------------------
			// One normal per face.
			// ---------------------------------------------------------

			mutable_u_normal[i] =
				ctx.makeUniform("u_face_normal", "vec3");
		}


		// -------------------------------------------------------------
		// Helper for one face.
		// -------------------------------------------------------------

		for (size_t i = 0; i < m_faceVertices.size(); ++i)
		{
			const auto& face = m_faceVertices[i];

			if (face.size() < 3)
				continue;

			ctx.append(
				"float sdf_face_" + std::to_string(i) +
				"(vec3 p) {"
			);

			ctx.append(
				"    vec3 n = normalize(" +
				mutable_u_normal[i] +
				");"
			);

			ctx.append(
				"    vec3 a = " +
				mutable_u_vertices[i][0] +
				";"
			);

			// ---------------------------------------------------------
			// Distance to plane.
			// ---------------------------------------------------------

			ctx.append(
				"    float planeDist = dot(p - a, n);"
			);

			ctx.append(
				"    vec3 q = p - planeDist * n;"
			);

			ctx.append(
				"    float edgeDist2 = 1e30;"
			);

			ctx.append(
				"    bool inside = true;"
			);


			// ---------------------------------------------------------
			// Edges.
			// ---------------------------------------------------------

			for (size_t j = 0; j < face.size(); ++j)
			{
				size_t next = (j + 1) % face.size();

				std::string aName =
					mutable_u_vertices[i][j];

				std::string bName =
					mutable_u_vertices[i][next];

				ctx.append(
					"    {"
				);

				ctx.append(
					"        vec3 va = " +
					aName +
					";"
				);

				ctx.append(
					"        vec3 vb = " +
					bName +
					";"
				);

				ctx.append(
					"        vec3 edge = vb - va;"
				);

				ctx.append(
					"        vec3 v = q - va;"
				);

				ctx.append(
					"        float edgeLen2 = dot(edge, edge);"
				);

				ctx.append(
					"        float t = 0.0;"
				);

				ctx.append(
					"        if (edgeLen2 > 1e-12)"
				);

				ctx.append(
					"            t = clamp(dot(v, edge) / edgeLen2, 0.0, 1.0);"
				);

				ctx.append(
					"        vec3 closest = va + t * edge;"
				);

				ctx.append(
					"        vec3 diff = q - closest;"
				);

				ctx.append(
					"        edgeDist2 = min(edgeDist2, dot(diff, diff));"
				);

				// -----------------------------------------------------
				// Convex polygon inside test.
				// -----------------------------------------------------

				ctx.append(
					"        if (dot(cross(edge, v), n) < -1e-6)"
				);

				ctx.append(
					"            inside = false;"
				);

				ctx.append(
					"    }"
				);
			}


			// ---------------------------------------------------------
			// Final distance.
			// ---------------------------------------------------------

			ctx.append(
				"    if (inside)"
			);

			ctx.append(
				"        return abs(planeDist);"
			);

			ctx.append(
				"    return sqrt(planeDist * planeDist + edgeDist2);"
			);

			ctx.append(
				"}"
			);
		}


		// -------------------------------------------------------------
		// Combine all faces.
		// -------------------------------------------------------------

		ctx.append(
			"float " + fname + "(vec3 p) {"
		);

		ctx.append(
			"    float d = 1e30;"
		);

		for (size_t i = 0; i < m_faceVertices.size(); ++i)
		{
			if (m_faceVertices[i].size() < 3)
				continue;

			ctx.append(
				"    d = min(d, sdf_face_" +
				std::to_string(i) +
				"(p));"
			);
		}

		ctx.append(
			"    return d;"
		);

		ctx.append(
			"}"
		);

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

		for (size_t i = 0; i < m_faceVertices.size(); ++i)
		{
			const auto& face = m_faceVertices[i];

			for (size_t j = 0; j < face.size(); ++j)
			{
				const Point& p = face[j];

				program.setUniformValue(
					mutable_u_vertices[i][j].c_str(),
					QVector3D(
						p[0],
						p[1],
						p[2]));
			}

			const Point& n = m_faceNormals[i];

			program.setUniformValue(
				mutable_u_normal[i].c_str(),
				QVector3D(
					n[0],
					n[1],
					n[2]));
		}
	}


	// -------------------------------------------------------------------------
	// Fragment shader
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
                    vec3 color =
                        vec3(1.0, 1.0, 0.0);

                    float alpha = 0.5;

                    FragColor =
                        vec4(color, alpha);


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
                t += max(abs(d) * 0.7, 0.001);


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

	// Original input representation
	Faces faces;

	// Actual polygon geometry
	std::vector<std::vector<Point>> m_faceVertices;

	// One oriented normal per face
	std::vector<Point> m_faceNormals;

	// Generated GLSL uniform names
	mutable std::vector<std::vector<std::string>>
		mutable_u_vertices;

	mutable std::vector<std::string>
		mutable_u_normal;
};