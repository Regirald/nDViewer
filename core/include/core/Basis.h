#pragma once

#include <array>
#include "Vec.h"
//by deflaut the basis is the standard orthonormal basis
template<size_t Dim>
class Basis : public std::array<Vec<Dim, float>, Dim>
{
public:

	Basis()
	{
		for (size_t i = 0; i < Dim; ++i) {
			Vec<Dim, float> vec;
			for (size_t j = 0; j < Dim; ++j) {
				if (i == j) {
					vec[j] = 1.0f;
				}
				else {
					vec[j] = 0.0f;
				}
			}
			this->at(i) = vec;
		}
	}
};
