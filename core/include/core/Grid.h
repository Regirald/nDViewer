#pragma once
#include <vector>
#include "Vec.h"
#include "Resolution.h"
#include "Basis.h"


template <size_t Dim, typename T>
class Grid {
public:
    using Index = Vec<Dim, int>;

    Grid(const Resolution<Dim>& resolution, Basis<Dim>& basis) 
        : m_resolution(resolution)
        , m_basis(basis) 
    {
        size_t total = 1;
        for (size_t i = 0; i < Dim; ++i)
            total *= m_resolution.size[i];
        m_data.resize(total);
    }

    T& operator()(const Index& idx) {
        return m_data[flatten(idx)];
    }

    const T& operator()(const Index& idx) const {
        return m_data[flatten(idx)];
    }

    const Index& getSize() const { return m_resolution.size; }
    const int getNbPoints() const { 
        int total = 1;
        for (size_t i = 0; i < Dim; ++i)
            total *= m_resolution.size[i];
        return total;
	}
    // Default position uses the underlying basis scaled by integer coords
    virtual Vec<Dim, T> getPosition(Vec<Dim, int> coords) const
    {
        Vec<Dim, T> p = {};
        for (int i = 0; i < static_cast<int>(Dim); ++i)
        {
            T scale = (static_cast<T>(coords[i]) / (m_resolution.size[i] - 1)) * 2.f - 1.f;
            p += this->m_basis[i] * scale;
        }
        return p;
    }

    size_t flatten(const Index& idx) const {
        size_t offset = 0, stride = 1;
        for (size_t i = 0; i < Dim; ++i) {
            offset += idx[i] * stride;
            stride *= m_resolution.size[i];
        }
        return offset;
    }

public:
    std::vector<T> m_data;
    Resolution<Dim> m_resolution;
    Basis<Dim> m_basis;

};

