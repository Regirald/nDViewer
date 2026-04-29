#pragma once
#include <vector>
#include "Vec.h"
#include "Resolution.h"
#include "Basis.h"


template <size_t Dim>
class Grid {
public:
    using Index = Vec<Dim, int>;
    using Point = Vec<Dim, float>;

    Grid(const Resolution<Dim>& resolution, Basis<Dim>& basis) 
        : m_resolution(resolution)
        , m_basis(basis) 
    {
    }

    const Index& getSize() const { return m_resolution.size; }
    const int getNbPoints() const { 
        int total = 1;
        for (size_t i = 0; i < Dim; ++i)
            total *= m_resolution.size[i];
        return total;
	}
    // Default position uses the underlying basis scaled by integer coords

    virtual Vec<Dim, float> getPosition(const Index& coords) const
    {
        Vec<Dim, float> p = {};
        for (size_t i = 0; i < Dim; ++i)
        {
            float scale = (static_cast<float>(coords[i]) / (m_resolution.size[i] - 1)) * 2.f - 1.f;
            p += this->m_basis[i] * scale;
        }
        return p;
    }

    virtual Vec<Dim, float> getPosition(const Point& pointBloc) const
    {
        Vec<Dim, float> p = {};
        for (size_t i = 0; i < Dim; ++i)
        {
            float scale = (static_cast<float>(pointBloc[i]) / (m_resolution.size[i] - 1)) * 2.f - 1.f;
            p += this->m_basis[i] * scale;
        }
        return p;
    }

    size_t flatten(const Index& idx) const 
    {
        size_t offset = 0, stride = 1;
        for (size_t i = 0; i < Dim; ++i) {
            offset += idx[i] * stride;
            stride *= m_resolution.size[i];
        }
        return offset;
    }

public:
    Resolution<Dim> m_resolution;
    Basis<Dim> m_basis;

};

