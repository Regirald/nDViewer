#pragma once
#include "Grid.h"
#include "Basis.h"

template <size_t Dim>
class ScalarField : public Grid<Dim>
{
	using Index = Vec<Dim, int>;
public:
	ScalarField(const Resolution<Dim>& size, Basis<Dim> basis)
		: Grid<Dim>(size, basis) 
	{
		size_t total = 1;
		for (size_t i = 0; i < Dim; ++i)
			total *= this->m_resolution.size[i];
		m_data.resize(total);
	}

	float& operator[](const Index& idx) {
		return m_data[this->flatten(idx)];
	}

	const float& operator[](const Index& idx) const {
		return m_data[this->flatten(idx)];
	}

protected:
	std::vector<float> m_data;
};