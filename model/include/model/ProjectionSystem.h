#pragma once
#include "core/Vec.h"



template<size_t Dim>
class ProjectionSystem
{
public:
    using Scalar = float;
    using Point = Vec<Dim, Scalar>;
    using PointHyperplane = Vec<Dim - 1, Scalar>;
    static_assert(Dim >= 2);


    virtual PointHyperplane project(const Point& p) const = 0;
};


template<size_t Dim>
class SchlegelProjection : public ProjectionSystem<Dim>
{
    using ProjectionSystem<Dim>::Scalar;
    using ProjectionSystem<Dim>::Point;
    using ProjectionSystem<Dim>::PointHyperplane;

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
    PointHyperplane project(const Point& p) const override
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


template class ProjectionSystem<2>;
template class ProjectionSystem<3>;
template class ProjectionSystem<4>;

template class SchlegelProjection<2>;
template class SchlegelProjection<3>;
template class SchlegelProjection<4>;
