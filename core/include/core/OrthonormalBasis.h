#pragma once
#include "Basis.h"

template<size_t Dim>
class OrthonormalBasis : public Basis<Dim>
{
public:
	OrthonormalBasis()
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
