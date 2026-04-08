#pragma once

#include <array>

template <size_t Dim, typename T>
struct Vec {
    std::array<T, Dim> data;

    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }
};

// Basic arithmetic operators for Vec to support simple geometry code.
template <size_t Dim, typename T>
inline Vec<Dim, T> operator+(const Vec<Dim, T>& a, const Vec<Dim, T>& b)
{
    Vec<Dim, T> r;
    for (size_t i = 0; i < Dim; ++i)
        r[i] = a[i] + b[i];
    return r;
}

template <size_t Dim, typename T>
inline Vec<Dim, T> operator-(const Vec<Dim, T>& a, const Vec<Dim, T>& b)
{
    Vec<Dim, T> r;
    for (size_t i = 0; i < Dim; ++i)
        r[i] = a[i] - b[i];
    return r;
}

template <size_t Dim, typename T>
inline Vec<Dim, T> operator*(const Vec<Dim, T>& v, T s)
{
    Vec<Dim, T> r;
    for (size_t i = 0; i < Dim; ++i)
        r[i] = v[i] * s;
    return r;
}

template <size_t Dim, typename T>
inline Vec<Dim, T> operator*(T s, const Vec<Dim, T>& v)
{
    return v * s;
}

// Compound assignment operators
template <size_t Dim, typename T>
inline Vec<Dim, T>& operator+=(Vec<Dim, T>& a, const Vec<Dim, T>& b)
{
    for (size_t i = 0; i < Dim; ++i)
        a[i] += b[i];
    return a;
}

template <size_t Dim, typename T>
inline Vec<Dim, T>& operator-=(Vec<Dim, T>& a, const Vec<Dim, T>& b)
{
    for (size_t i = 0; i < Dim; ++i)
        a[i] -= b[i];
    return a;
}

template <size_t Dim, typename T>
inline Vec<Dim, T>& operator*=(Vec<Dim, T>& v, T s)
{
    for (size_t i = 0; i < Dim; ++i)
        v[i] *= s;
    return v;
}

template <size_t Dim, typename T>
inline Vec<Dim, T>& operator/=(Vec<Dim, T>& v, T s)
{
    for (size_t i = 0; i < Dim; ++i)
        v[i] /= s;
    return v;
}
