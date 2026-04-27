#pragma once

#include <array>
#include "Vec.h"
//by deflaut the basis is the standard orthonormal basis
template<size_t Dim>
class Basis : public std::array<Vec<Dim, float>, Dim>
{
public:


};
