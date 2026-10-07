#include "RenderPolytope.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QVector3D>
#include "Renderer/VerticesRenderer.h"
#include "Renderer/EdgesRenderer.h"
#include "Renderer/FacesRenderer.h"
#include <memory>
#include <unordered_map>

// ------------------------------------------------------------
// Constructor (NO OpenGL here)
// ------------------------------------------------------------
RenderPolytope::RenderPolytope(PolytopeGeometry3D* polytope)
	: m_polytope(polytope)
{
	vbo = 0;
	eboEdges = 0;
	eboFaces = 0;
	gl = nullptr;
	m_initialized = false;
}

// ------------------------------------------------------------
// GPU initialization (called lazily when context is valid)
// ------------------------------------------------------------
void RenderPolytope::initializeGLResources()
{
	if (!m_polytope)
		return;

	QOpenGLContext* ctx = QOpenGLContext::currentContext();
	gl = ctx->extraFunctions();
	gl->initializeOpenGLFunctions();

	// -------------------------
	// 1. Compile shader
	// -------------------------
	if (!m_program.isLinked())
	{
		const char* vs =
			"#version 330 core\n"
			"layout(location = 0) in vec3 a_position;\n"
			"uniform mat4 u_view;\n"
			"uniform mat4 u_projection;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = u_projection * u_view * vec4(a_position, 1.0);\n"
			"}\n";

		const char* fs =
			"#version 330 core\n"
			"out vec4 FragColor;\n"
			"uniform vec3 u_color;\n"
			"void main()\n"
			"{\n"
			"    FragColor = vec4(u_color, 1.0);\n"
			"}\n";

		m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
		m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
		m_program.link();
	}

	// -------------------------
	// 2. Flatten vertex data
	// -------------------------
	std::vector<float> flatVertices;
	flatVertices.reserve(m_polytope->vertices.size() * 3);

	for (const auto& v : m_polytope->vertices)
	{
		flatVertices.push_back(v[0]);
		flatVertices.push_back(v[1]);
		flatVertices.push_back(v[2]);
	}

	// Build indices for edges (pairs)
	std::vector<uint32_t> edgeIndices;
	for (const auto& e : m_polytope->edges)
	{
		if (e.size() >= 2)
		{
			edgeIndices.push_back(static_cast<uint32_t>(e[0]));
			edgeIndices.push_back(static_cast<uint32_t>(e[1]));
		}
	}

	m_indexCount = static_cast<int>(edgeIndices.size());

	// -------------------------
	// Build face indices (triangulate polygonal faces)
	// -------------------------
	std::vector<uint32_t> faceIndices;
	for (const auto& f : m_polytope->faces)
	{
		if (f.size() < 3)
			continue;
		// fan triangulation: (0,i,i+1)
		for (size_t i = 0; i + 2 < f.size(); ++i)
		{
			faceIndices.push_back(static_cast<uint32_t>(f[i]));
			faceIndices.push_back(static_cast<uint32_t>(f[i + 1]));
			faceIndices.push_back(static_cast<uint32_t>(f[i + 2]));
		}
	}

	m_faceIndexCount = static_cast<int>(faceIndices.size());

	// -------------------------
	// 3. Create VAO FIRST
	// -------------------------
	gl->glGenVertexArrays(1, &vao);
	gl->glBindVertexArray(vao);

	// -------------------------
	// 4. Create & fill VBO
	// -------------------------
	gl->glGenBuffers(1, &vbo);
	gl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
	gl->glBufferData(GL_ARRAY_BUFFER,
		flatVertices.size() * sizeof(float),
		flatVertices.data(),
		GL_STATIC_DRAW);

	// -------------------------
	// 5. Create & fill EBO for edges
	// -------------------------
	gl->glGenBuffers(1, &eboEdges);
	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboEdges);
	gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		edgeIndices.size() * sizeof(uint32_t),
		edgeIndices.data(),
		GL_STATIC_DRAW);

	// -------------------------
	// 6. Create & fill EBO for faces
	// -------------------------
	gl->glGenBuffers(1, &eboFaces);
	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboFaces);
	gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		faceIndices.size() * sizeof(uint32_t),
		faceIndices.data(),
		GL_STATIC_DRAW);

	// -------------------------
	// 7. Define vertex layout
	// -------------------------
	gl->glEnableVertexAttribArray(0);
	gl->glVertexAttribPointer(
		0,              // location = 0
		3,              // vec3
		GL_FLOAT,
		GL_FALSE,
		3 * sizeof(float),
		(void*)0
	);

	// -------------------------
	// 7. Unbind VAO (important)
	// -------------------------
	gl->glBindVertexArray(0);

	m_initialized = true;
}

// ------------------------------------------------------------
// Render
// ------------------------------------------------------------
void RenderPolytope::render(const ndv::RenderContext& context)
{
	if (!m_initialized)
		initializeGLResources();

	if (!m_initialized)
		return;

	// Delegate rendering of components to specialized renderers (3D only)

		static std::unordered_map<RenderPolytope*, std::unique_ptr<VerticesRenderer>> s_verticesMap;
		static std::unordered_map<RenderPolytope*, std::unique_ptr<EdgesRenderer>> s_edgesMap;
		static std::unordered_map<RenderPolytope*, std::unique_ptr<FacesRenderer>> s_facesMap;

		// ensure renderers are initialized for this instance
		if (s_verticesMap.find(this) == s_verticesMap.end())
		{
			s_verticesMap[this] = std::make_unique<VerticesRenderer>(static_cast<PolytopeGeometry3D*>(m_polytope));
			s_verticesMap[this]->initializeGLResources();
		}

		if (s_edgesMap.find(this) == s_edgesMap.end())
		{
			s_edgesMap[this] = std::make_unique<EdgesRenderer>(static_cast<PolytopeGeometry3D*>(m_polytope));
			s_edgesMap[this]->initializeGLResources();
		}

		if (s_facesMap.find(this) == s_facesMap.end())
		{
			s_facesMap[this] = std::make_unique<FacesRenderer>(static_cast<PolytopeGeometry3D*>(m_polytope));
			s_facesMap[this]->initializeGLResources();
		}

		// Render each component. Faces rendering can be toggled off for performance.
		s_verticesMap[this]->render(context);
		s_edgesMap[this]->render(context);
		if (context.fullShadedRendering)
		{
			s_facesMap[this]->render(context);
		}
	
}

// ------------------------------------------------------------
// Destructor (cleanup)
// ------------------------------------------------------------
RenderPolytope::~RenderPolytope()
{
	if (!gl)
		return;

	if (vbo) gl->glDeleteBuffers(1, &vbo);
	if (eboEdges) gl->glDeleteBuffers(1, &eboEdges);
	if (eboFaces) gl->glDeleteBuffers(1, &eboFaces);
	if (vao) gl->glDeleteVertexArrays(1, &vao);
}


