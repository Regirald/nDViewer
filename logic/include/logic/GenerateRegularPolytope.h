#pragma once
#include <vector>
#include "model/Mesh.h"
#include "model/CoxeterPresentation.h"
#include "model/ToddCoxeter.h"
#include "model/GeometryFromTopology.h"

class RegularPolytopeGenerator
{
public:
    static AbstractPolytopeGeometry* generateRegularPolytope(std::vector<int>& schlafli);

    template <size_t Dim>
    static PolytopeGeometry<Dim> generateRegularPolytopeFromSimplifiedSchlafli(const std::vector<int>& schlafli_simplified);

    static const std::vector<int> simplifySchlafli(const std::vector<int>& schlafli);
};


