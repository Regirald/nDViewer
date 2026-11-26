#pragma once
#include "core/Vec.h"
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <QVector3D>

class PolytopeNodeLight {
public:
	using Point = Vec<3, float>;

	// Each node is defined by a position and a m_radius
	PolytopeNodeLight(const Point& node, float radius)
		: node(node), radius(radius)
	{}


	Point node;
	float radius;
};
