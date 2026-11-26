#pragma once

#include "ImplicitFunctionSet.h"
#include "ImplicitFunction.h"
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

class VerticeSet : public ImplicitFunctionSet<3>
{
public:
	using Point = typename ImplicitFunctionSet<3>::Point;



	VerticeSet(std::vector<Point> vertices, float radius);
	~VerticeSet() override;


	float evaluateSDF(const Point& p) const override;

	void setGLUniforms(QOpenGLShaderProgram& program, int indexCurrentPrimitive) const override;
	const std::string getFragmentShader() const override;
	const std::string getVertexShader() const override;

	void initializeVerticeSetBuffer() const;

	std::vector<Point> vertices;
	float radius;
	mutable GLuint m_verticeSetBuffer = 0;

};

