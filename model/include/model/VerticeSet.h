#pragma once

#include "ImplicitFunction.h"
#include "PolytopeShader.h"
#include "AccelerationStructureBVH.h"

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

class VerticeSet : public ImplicitFunction<3>
{
public:
	using Point = typename ImplicitFunction<3>::Point;
	using Node = typename AccelerationStructureBVH::Node;
	using Primitive = typename AccelerationStructureBVH::Primitive;



	VerticeSet(
		std::vector<Point> vertices,
		std::vector<Node> nodes,
		std::vector<Primitive> primitives,
		float radius);

	~VerticeSet() override;

	float evaluateSDF(const Point& p) const override;

	std::string buildGLSL(GLSLContext& ctx) const override;

	void setGLUniforms(QOpenGLShaderProgram& program) const override;

	const std::string getFragmentShader() const override;

	std::vector<Point> vertices;
	std::vector<Node> nodes;
	std::vector<Primitive> primitives;
	float radius;

private:
	mutable GLuint m_bvhBuffer = 0;
	mutable GLuint m_bvhTexture = 0;
	std::vector<float> m_packedData;

	void packBVHData();

	void ensureBVHBuffer() const;
};

