#pragma once

#include "Grid.h"
#include "Vec.h"
#include "SimplexBasis.h"

template <size_t Dim, typename T>
class SimplexGrid : public Grid<Dim, T> {

public:
    SimplexGrid(const Resolution<Dim>& size, SimplexBasis<Dim> basis)
        : Grid<Dim, T>(size, basis) 
        , m_simplexBasis(basis)
    {
        m_simplexBasis.build();
    }

    Vec<Dim, T> getPosition(Vec<Dim, int> coords) const override;

    SimplexBasis<Dim> m_simplexBasis;
};

// inline/template definition must be in header
template <size_t Dim, typename T>
inline Vec<Dim, T> SimplexGrid<Dim, T>::getPosition(Vec<Dim, int> coords) const
{
    Vec<Dim, T> p = {};

    // simple default: use basis vectors scaled by integer coords
    for (int i = 0; i < Dim; ++i)
        p += m_simplexBasis[i] * static_cast<T>(coords[i]);

    return p;
}
