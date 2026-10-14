#pragma once
#include "ImplicitFunction.h"
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <QVector3D>

class PolytopeNodeLight {
public:
	using Point = ImplicitFunction<3>::Point;

	// Each node is defined by a position and a radius
	PolytopeNodeLight(const Point& node, float radius)
		: node(node), radius(radius)
	{}


	Point node;
	float radius;
};
