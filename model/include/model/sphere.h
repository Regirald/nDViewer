#pragma once
#include "ImplicitFunction.h"


class Sphere : public ImplicitFunction<3> {
public:
    float evaluate(const Point& p) const override {
        return std::sqrt((p[0] - center[0]) * (p[0] - center[0]) +
                         (p[1] - center[1]) * (p[1] - center[1]) +
                         (p[2] - center[2]) * (p[2] - center[2])) -
               radius;
    }

	Vec<3, float> center = { 0.f, 0.f, 0.f };
	float radius = 1.f;

};