#pragma once
#include "Grid.h"

template <size_t Dim, typename T>
class OrthonormalGrid : public Grid<Dim> {


	public:
	OrthonormalGrid(const Resolution<Dim>& size, Basis<Dim> basis)
		: Grid<Dim>(size, basis) 
	{
	}
	Vec<Dim, T> getPosition(Vec<Dim, int> coords) const override;
};