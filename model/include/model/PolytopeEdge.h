#pragma once

#include "ImplicitFunctionSet.h"
#include "PolytopeShaderBuilder.h"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

#include <string>
#include <vector>

class PolytopeEdge : public ImplicitFunctionSet<3>
{
public:
	using Point = typename ImplicitFunctionSet<3>::Point;

	PolytopeEdge(std::vector<Point> vertices_a, std::vector<Point> vertices_b, float radius);
	~PolytopeEdge() override;

	float evaluateSDF(const Point& p) const override;
	void setGLUniforms(QOpenGLShaderProgram& program, int indexCurrentPrimitive) const override;
	const std::string getFragmentShader() const override;
	const std::string getVertexShader() const override;

	void initializePolytopeEdgeBuffer() const;

	std::vector<Point> m_vertices_a;
	std::vector<Point> m_vertices_b;
	float m_radius;
	mutable GLuint m_polytopeEdgeBuffer = 0;
};
