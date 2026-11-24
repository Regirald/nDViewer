#pragma once

#include "ImplicitFunction.h"
#include "PolytopeShader.h"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QVector3D>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

template <size_t Dim>
class VerticeSet : public ImplicitFunction<Dim>
{
public:
	using Point = typename ImplicitFunction<Dim>::Point;



	VerticeSet(
		std::vector<Point> vertices,
		float radius);


	float evaluateSDF(const Point& p) const override;

	std::string buildGLSL(GLSLContext& ctx) const override;

	void setGLUniforms(QOpenGLShaderProgram& program) const override;

	const std::string getFragmentShader() const override;

	std::vector<Point> vertices;
	float radius;

};

