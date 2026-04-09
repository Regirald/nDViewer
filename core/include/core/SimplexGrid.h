#pragma once

#include "Grid.h"
#include "Vec.h"
#include "SimplexBasis.h"

template <size_t Dim, typename T>
class SimplexGrid : public Grid<Dim, T> {

public:
    SimplexGrid(const Resolution<Dim>& size, SimplexBasis<Dim> basis)
        : Grid<Dim, T>(size, basis) 
    {
    }

    Vec<Dim, T> getPosition(Vec<Dim, int> coords) const override;

};

template <size_t Dim, typename T>
inline Vec<Dim, T> SimplexGrid<Dim, T>::getPosition(Vec<Dim, int> coords) const
{
    Vec<Dim, T> p = {};

    // simple default: use basis vectors scaled by integer coords
    for (int i = 0; i < Dim; ++i)
    {
        T scale = (static_cast<T>(coords[i]) / (this->m_resolution.size[i] - 1)) * 2.f - 1.f;
        p += this->m_basis[i] * scale;
    }
    return p;
}
