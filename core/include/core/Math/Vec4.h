#pragma once

#include <ostream>

namespace ndv::math
{
    struct Vec4
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 1.0f;

        Vec4() = default;
        Vec4(float xValue, float yValue, float zValue, float wValue) : x(xValue), y(yValue), z(zValue), w(wValue) {}

        float& operator[](size_t index)
        {
            return index == 0 ? x : (index == 1 ? y : (index == 2 ? z : w));
        }

        const float& operator[](size_t index) const
        {
            return index == 0 ? x : (index == 1 ? y : (index == 2 ? z : w));
        }
    };

    inline Vec4 operator+(const Vec4& a, const Vec4& b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
    inline Vec4 operator-(const Vec4& a, const Vec4& b) { return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
    inline Vec4 operator*(const Vec4& v, float s) { return { v.x * s, v.y * s, v.z * s, v.w * s }; }
    inline Vec4 operator*(float s, const Vec4& v) { return v * s; }

    inline std::ostream& operator<<(std::ostream& os, const Vec4& v)
    {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }
}
