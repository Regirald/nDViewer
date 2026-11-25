#pragma once

#include "ImplicitFunctionSet.h"
#include "PolytopeShaderBuilder.h"

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
class VerticeSet : public ImplicitFunctionSet<Dim>  // TODO : IMPLICITSSSS FUNCTIONSSS
{
public:
	using Point = typename ImplicitFunctionSet<Dim>::Point;



	VerticeSet(std::vector<Point> vertices, float radius);


	float evaluateSDF(const Point& p) const override;
	const std::string buildSDF() const override;
	const std::string buildUniforms() const override;
	const std::string getFragmentShader() const override;
	const std::string getVertexShader() const override;
	void setGLUniforms(QOpenGLShaderProgram& program) const override;


	std::vector<Point> vertices;
	float radius;

};

