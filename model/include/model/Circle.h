#pragma once
#include "ImplicitFunction.h"
#include <cmath>

class Circle : public ImplicitFunction<2> {
public:
    float evaluateSDF(const Point& p) const override {
        return sqrt(p[0] * p[0] + p[1] * p[1]) - 1.0f;
    }
};