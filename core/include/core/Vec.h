#pragma once

#include <array>

template <size_t Dim, typename T>
struct Vec {
    std::array<T, Dim> data;

    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }
};