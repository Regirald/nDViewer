#pragma once
#include "ImplicitFunction.h"


class Circle : public ImplicitFunction<2> {
public:
    float evaluate(const Point& p) const override {
        return p[0] * p[0] + p[1] * p[1] - 1.0f;
    }
};