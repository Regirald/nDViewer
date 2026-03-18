#include "ImplicitFunction.h"

class Sphere : public ImplicitFunction<3> {
public:
    float evaluate(const Point& p) const override {
        return p[0] * p[0] + p[1] * p[1] + p[2] * p[2] - 1.0f;
    }
};