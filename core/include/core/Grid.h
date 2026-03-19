#pragma once
#include "Vec.h"

template <size_t Dim, typename T>
class Grid {
public:
    using Index = Vec<Dim, int>;

    Grid(const Index& size) : m_size(size) {
        size_t total = 1;
        for (size_t i = 0; i < Dim; ++i)
            total *= size[i];
        m_data.resize(total);
    }

    T& operator()(const Index& idx) {
        return m_data[flatten(idx)];
    }

    const T& operator()(const Index& idx) const {
        return m_data[flatten(idx)];
    }

    const Index& getSize() const { return m_size; }

private:
    std::vector<T> m_data;
    Index m_size;

    size_t flatten(const Index& idx) const {
        size_t offset = 0, stride = 1;
        for (size_t i = 0; i < Dim; ++i) {
            offset += idx[i] * stride;
            stride *= m_size[i];
        }
        return offset;
    }
};