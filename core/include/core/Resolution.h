#pragma once

#include "Vec.h"

template <size_t Dim>
class Resolution
{
public:
    Vec<Dim, int> size;

    Resolution(const Vec<Dim, int>& s) : size(s) {}

    int totalSize() const {
        int t = 1;
        for (size_t i = 0; i < Dim; ++i)
            t *= size[i];
        return t;
    }
};