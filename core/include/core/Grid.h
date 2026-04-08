#pragma once
#include <vector>
#include "Vec.h"
#include "Resolution.h"
#include "Basis.h"


template <size_t Dim, typename T>
class Grid {
public:
    using Index = Vec<Dim, int>;

    Grid(const Resolution<Dim>& resolution, Basis<Dim> basis) 
        : m_size(resolution.size)
        , m_basis(basis) 
    {
        size_t total = 1;
        for (size_t i = 0; i < Dim; ++i)
            total *= m_size[i];
        m_data.resize(total);
		m_basis.build();
    }

    T& operator()(const Index& idx) {
        return m_data[flatten(idx)];
    }

    const T& operator()(const Index& idx) const {
        return m_data[flatten(idx)];
    }

    const Index& getSize() const { return m_size; }
    const int getNbPoints() const { 
        int total = 1;
        for (size_t i = 0; i < Dim; ++i)
            total *= m_size[i];
        return total;
	}
    // Default position uses the underlying basis scaled by integer coords
    virtual Vec<Dim, T> getPosition(Vec<Dim, int> coords) const
    {
        Vec<Dim, T> p = {};
        for (int i = 0; i < static_cast<int>(Dim); ++i)
            p += this->m_basis[i] * static_cast<T>(coords[i]);
        return p;
    }

    size_t flatten(const Index& idx) const {
        size_t offset = 0, stride = 1;
        for (size_t i = 0; i < Dim; ++i) {
            offset += idx[i] * stride;
            stride *= m_size[i];
        }
        return offset;
    }

protected:
    std::vector<T> m_data;
    Index m_size;
    Basis<Dim> m_basis;

};

