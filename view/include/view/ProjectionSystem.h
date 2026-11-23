#pragma once
#include "core/Vec.h"

template<size_t N>
class SchlegelProjection
{
    static_assert(N >= 2);

    using Scalar = float;
    using Point = Vec<N, Scalar>;
    using PointHyperplane = Vec<N - 1, Scalar>;

public:

    // Construct from a facet hyperplane and projection center
    constexpr SchlegelProjection(
        const Point& facetNormal,
        Scalar facetOffset,
        const Point& projectionCenter)
        : normal(facetNormal),
        offset(facetOffset),
        center(projectionCenter)
    {
    }

    // Project an N-dimensional point onto the facet hyperplane
    constexpr PointHyperplane Project(const Point& p) const
    {
        const Scalar denominator = dot(normal, p - center);

        const Scalar t = (offset - dot(normal, center)) / denominator;

		Point projectedPoint = center + (p - center) * t;

		PointHyperplane projectedPointHyperplane(projectedPoint); // crop last coordinate to get the projected point in the hyperplane

        return projectedPointHyperplane;
    }

    // Hyperplane of the selected facet:
    // dot(normal, x) = offset
    Point normal;
    Scalar offset;

    // Projection center (outside the polytope)
    Point center;
};


 template class SchlegelProjection<2>;
 template class SchlegelProjection<3>;
 template class SchlegelProjection<4>;
