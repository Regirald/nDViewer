#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "core/Vec.h"
#include "CoxeterPresentation.h"

using Matrix = std::vector<std::vector<float>>;
using Vector = std::vector<float>;

Matrix cholesky(const Matrix& G);

Matrix buildGramMatrix(const Coxeter::Presentation& presentation);
Vector buildV0(const Matrix& N);


template<size_t Dim, typename T>
inline std::pair<Vec<Dim, T>, std::vector<Vec<Dim, float>>> buildV0andMirrors(const Coxeter::Presentation& presentation)
{
	Matrix G = buildGramMatrix(presentation);
	Matrix L = cholesky(G);
	Vector V0 = buildV0(L);
	Vec<Dim, T> v0;
	for (int i = 0; i < presentation.rank; ++i)
	{
		v0[i] = static_cast<T>(V0[i]);
	}
	std::vector<Vec<Dim, float>> mirrors;
	for (int i = 0; i < presentation.rank; ++i)
	{
		Vec<Dim, float> normal;
		for (int j = 0; j < presentation.rank; ++j)
		{
			normal[j] = static_cast<float>(L[i][j]);
		}
		mirrors.push_back(normal);
	}

	return std::pair<Vec<Dim, T>, std::vector<Vec<Dim, float>>>(v0, mirrors);
}

