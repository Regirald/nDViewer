#pragma once
#include "core/Vec.h"

template <size_t Dim>
class ImplicitFunction {
public:
    using Point = Vec<Dim, float>;

    virtual float evaluate(const Point& p) const = 0;
};