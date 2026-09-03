#pragma once
#include <vector>
#include "model/Mesh.h"
#include "model/CoxeterPresentation.h"
#include "model/ToddCoxeter.h"
#include "model/GramMatrixNormals.h"
#include "model/GeometryFromTopology.h"

template <size_t Dim>
class RegularPolytopeGenerator
{
public:
    static Mesh<Dim> generateRegularPolytope(const std::vector<int>& schlafli);

};


