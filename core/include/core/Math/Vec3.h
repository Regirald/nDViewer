#pragma once

#include <cmath>
#include <ostream>

namespace ndv::math
{
    struct Vec2
    {
        float x = 0.0f;
        float y = 0.0f;

        Vec2() = default;
        Vec2(float xValue, float yValue) : x(xValue), y(yValue) {}

        float& operator[](size_t index) { return index == 0 ? x : y; }
        const float& operator[](size_t index) const { return index == 0 ? x : y; }
    };

    inline Vec2 operator+(const Vec2& a, const Vec2& b) { return { a.x + b.x, a.y + b.y }; }
    inline Vec2 operator-(const Vec2& a, const Vec2& b) { return { a.x - b.x, a.y - b.y }; }
    inline Vec2 operator*(const Vec2& v, float s) { return { v.x * s, v.y * s }; }
    inline Vec2 operator*(float s, const Vec2& v) { return v * s; }

    inline std::ostream& operator<<(std::ostream& os, const Vec2& v)
    {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }

    struct Vec3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vec3() = default;
        Vec3(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}

        float& operator[](size_t index)
        {
            return index == 0 ? x : (index == 1 ? y : z);
        }

        const float& operator[](size_t index) const
        {
            return index == 0 ? x : (index == 1 ? y : z);
        }

        float length() const
        {
            return std::sqrt(x * x + y * y + z * z);
        }

        Vec3 normalized() const
        {
            const float len = length();
            if (len == 0.0f)
            {
                return {};
            }
            return { x / len, y / len, z / len };
        }
    };

    inline Vec3 operator+(const Vec3& a, const Vec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
    inline Vec3 operator-(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
    inline Vec3 operator*(const Vec3& v, float s) { return { v.x * s, v.y * s, v.z * s }; }
    inline Vec3 operator*(float s, const Vec3& v) { return v * s; }

    inline float dot(const Vec3& a, const Vec3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline Vec3 cross(const Vec3& a, const Vec3& b)
    {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    inline std::ostream& operator<<(std::ostream& os, const Vec3& v)
    {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
}
