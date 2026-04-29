#pragma once

#include "Grid.h"
#include "Vec.h"
#include "SimplexBasis.h"

template <size_t Dim>
class SimplexGrid : public Grid<Dim> {

public:
    SimplexGrid(const Resolution<Dim>& size, Basis<Dim> basis)
        : Grid<Dim>(size, basis) 
    {
    }

    Vec<Dim, float> getPosition(const typename Grid<Dim>::Index& coords) const override;

};

template <size_t Dim>
inline Vec<Dim, float> SimplexGrid<Dim>::getPosition(const typename Grid<Dim>::Index& coords) const
{
    Vec<Dim, float> p = {};

    // simple default: use basis vectors scaled by integer coords
    for (size_t i = 0; i < Dim; ++i)
    {
        float scale = (static_cast<float>(coords[i]) / (this->m_resolution.size[i] - 1)) * 2.f - 1.f;
        p += this->m_basis[i] * scale;
    }
    return p;
}
