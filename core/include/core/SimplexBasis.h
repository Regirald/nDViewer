#pragma once

#include <cmath>
#include "Basis.h"

// SimplexBasis is a template; its implementation must be available in the header
template<size_t Dim>
class SimplexBasis : public Basis<Dim>
{
public:
	SimplexBasis() 
	{
		// Construct an orthogonal-ish simplex basis in R^Dim
     for (int i = 0; i < static_cast<int>(Dim); ++i)
		{
			Vec<Dim, float> v = {};
			for (int j = 0; j < i; ++j)
				v[j] = 0.5f;
			v[i] = std::sqrt(1.0f - 0.25f * i);
			this->at(i) = v;
		}
	}
};

