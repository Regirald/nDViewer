#pragma once

#include <array>
#include <iostream>

template <size_t Dim, typename T>
struct Vec {

    Vec() {
        data.fill(T(0));
	}
	Vec(T v) {
        data.fill(v);
    }
	Vec(std::initializer_list<T> list) {
        size_t i = 0;
        for (T v : list) {
            if (i < Dim) {
                data[i++] = v;
            }
        }
        for (; i < Dim; ++i) {
            data[i] = T(0);
        }
    }
    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }

    std::array<T, Dim> data;
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

template <size_t Dim, typename T>
inline Vec<Dim, T> operator/(const Vec<Dim, T>& v, T s)
{
    Vec<Dim, T> r;
    for (size_t i = 0; i < Dim; ++i)
        r[i] = v[i] / s;
    return r;
}

template <size_t Dim, typename T>
inline Vec<Dim, T> operator/(T s, const Vec<Dim, T>& v)
{
    return v / s;
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

// Stream output for debugging Vec values, prints as "(v0, v1, ..., vN)".
template <size_t Dim, typename T>
inline std::ostream& operator<<(std::ostream& os, const Vec<Dim, T>& v)
{
    os << "(";
    for (size_t i = 0; i < Dim; ++i) {
        if (i) os << ", ";
        os << v[i];
    }
    os << ")";
    return os;
}

template <size_t Dim, typename T>
double dot(const Vec<Dim, T>& a, const Vec<Dim, T>& b)
{
    double r = 0.0;
    for (int i = 0; i < Dim; ++i)
        r += a[i] * b[i];
    return r;
}

template <size_t Dim, typename T>
double lenght(const Vec<Dim, T>& v)
{
    return std::sqrt(dot(v, v));
}

template <size_t Dim, typename T>
Vec<Dim, T> normalize(const Vec<Dim, T>& v)
{
    double n = lenght(v);

    Vec<Dim, T> r;

    for (int i = 0; i < Dim; ++i)
        r[i] = v[i] / n;

    return r;
}

template <size_t Dim, typename T>
Vec<Dim, T> square(const Vec<Dim, T>& v)
{
    Vec<Dim, T> r;
    for (size_t i = 0; i < Dim; ++i)
        r[i] = v[i] * v[i];
    return r;
}