#pragma once

#include <cmath>
#include <cstring>

#include "core/Math/Vec3.h"
#include "core/Math/Vec4.h"

namespace ndv::math
{
    struct Mat4
    {
        // Column-major storage to match OpenGL conventions.
        float m[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        static Mat4 identity()
        {
            return {};
        }

        float& operator()(size_t row, size_t col)
        {
            return m[col * 4 + row];
        }

        const float& operator()(size_t row, size_t col) const
        {
            return m[col * 4 + row];
        }

        static Mat4 translation(const Vec3& t)
        {
            Mat4 result = identity();
            result(0, 3) = t.x;
            result(1, 3) = t.y;
            result(2, 3) = t.z;
            return result;
        }

        static Mat4 scale(const Vec3& s)
        {
            Mat4 result = identity();
            result(0, 0) = s.x;
            result(1, 1) = s.y;
            result(2, 2) = s.z;
            return result;
        }

        static Mat4 rotationX(float radians)
        {
            Mat4 result = identity();
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            result(1, 1) = c;
            result(2, 2) = c;
            result(1, 2) = -s;
            result(2, 1) = s;
            return result;
        }

        static Mat4 rotationY(float radians)
        {
            Mat4 result = identity();
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            result(0, 0) = c;
            result(2, 2) = c;
            result(0, 2) = s;
            result(2, 0) = -s;
            return result;
        }

        static Mat4 rotationZ(float radians)
        {
            Mat4 result = identity();
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            result(0, 0) = c;
            result(1, 1) = c;
            result(0, 1) = -s;
            result(1, 0) = s;
            return result;
        }

        static Mat4 perspective(float fovYRadians, float aspect, float nearPlane, float farPlane)
        {
            Mat4 result = {};
            const float f = 1.0f / std::tan(fovYRadians * 0.5f);
            result(0, 0) = f / aspect;
            result(1, 1) = f;
            result(2, 2) = (farPlane + nearPlane) / (nearPlane - farPlane);
            result(2, 3) = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
            result(3, 2) = -1.0f;
            result(3, 3) = 0.0f;
            return result;
        }

        static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
        {
            const Vec3 forward = (target - eye).normalized();
            const Vec3 right = cross(forward, up).normalized();
            const Vec3 correctedUp = cross(right, forward);

            Mat4 result = identity();
            result(0, 0) = right.x;
            result(1, 0) = right.y;
            result(2, 0) = right.z;
            result(0, 1) = correctedUp.x;
            result(1, 1) = correctedUp.y;
            result(2, 1) = correctedUp.z;
            result(0, 2) = -forward.x;
            result(1, 2) = -forward.y;
            result(2, 2) = -forward.z;
            result(0, 3) = -dot(right, eye);
            result(1, 3) = -dot(correctedUp, eye);
            result(2, 3) = dot(forward, eye);
            return result;
        }

        Mat4 transposed() const
        {
            Mat4 result = {};
            for (size_t row = 0; row < 4; ++row)
            {
                for (size_t col = 0; col < 4; ++col)
                {
                    result(row, col) = (*this)(col, row);
                }
            }
            return result;
        }

        Mat4 inverted() const
        {
            Mat4 inv = {};
            const float* a = m;
            float* invOut = inv.m;

            invOut[0] = a[5] * a[10] * a[15] - a[5] * a[11] * a[14] - a[9] * a[6] * a[15]
                + a[9] * a[7] * a[14] + a[13] * a[6] * a[11] - a[13] * a[7] * a[10];
            invOut[4] = -a[4] * a[10] * a[15] + a[4] * a[11] * a[14] + a[8] * a[6] * a[15]
                - a[8] * a[7] * a[14] - a[12] * a[6] * a[11] + a[12] * a[7] * a[10];
            invOut[8] = a[4] * a[9] * a[15] - a[4] * a[11] * a[13] - a[8] * a[5] * a[15]
                + a[8] * a[7] * a[13] + a[12] * a[5] * a[11] - a[12] * a[7] * a[9];
            invOut[12] = -a[4] * a[9] * a[14] + a[4] * a[10] * a[13] + a[8] * a[5] * a[14]
                - a[8] * a[6] * a[13] - a[12] * a[5] * a[10] + a[12] * a[6] * a[9];
            invOut[1] = -a[1] * a[10] * a[15] + a[1] * a[11] * a[14] + a[9] * a[2] * a[15]
                - a[9] * a[3] * a[14] - a[13] * a[2] * a[11] + a[13] * a[3] * a[10];
            invOut[5] = a[0] * a[10] * a[15] - a[0] * a[11] * a[14] - a[8] * a[2] * a[15]
                + a[8] * a[3] * a[14] + a[12] * a[2] * a[11] - a[12] * a[3] * a[10];
            invOut[9] = -a[0] * a[9] * a[15] + a[0] * a[11] * a[13] + a[8] * a[1] * a[15]
                - a[8] * a[3] * a[13] - a[12] * a[1] * a[11] + a[12] * a[3] * a[9];
            invOut[13] = a[0] * a[9] * a[14] - a[0] * a[10] * a[13] - a[8] * a[1] * a[14]
                + a[8] * a[2] * a[13] + a[12] * a[1] * a[10] - a[12] * a[2] * a[9];
            invOut[2] = a[1] * a[6] * a[15] - a[1] * a[7] * a[14] - a[5] * a[2] * a[15]
                + a[5] * a[3] * a[14] + a[13] * a[2] * a[7] - a[13] * a[3] * a[6];
            invOut[6] = -a[0] * a[6] * a[15] + a[0] * a[7] * a[14] + a[4] * a[2] * a[15]
                - a[4] * a[3] * a[14] - a[12] * a[2] * a[7] + a[12] * a[3] * a[6];
            invOut[10] = a[0] * a[5] * a[15] - a[0] * a[7] * a[13] - a[4] * a[1] * a[15]
                + a[4] * a[3] * a[13] + a[12] * a[1] * a[7] - a[12] * a[3] * a[5];
            invOut[14] = -a[0] * a[5] * a[14] + a[0] * a[6] * a[13] + a[4] * a[1] * a[14]
                - a[4] * a[2] * a[13] - a[12] * a[1] * a[6] + a[12] * a[2] * a[5];
            invOut[3] = -a[1] * a[6] * a[11] + a[1] * a[7] * a[10] + a[5] * a[2] * a[11]
                - a[5] * a[3] * a[10] - a[9] * a[2] * a[7] + a[9] * a[3] * a[6];
            invOut[7] = a[0] * a[6] * a[11] - a[0] * a[7] * a[10] - a[4] * a[2] * a[11]
                + a[4] * a[3] * a[10] + a[8] * a[2] * a[7] - a[8] * a[3] * a[6];
            invOut[11] = -a[0] * a[5] * a[11] + a[0] * a[7] * a[9] + a[4] * a[1] * a[11]
                - a[4] * a[3] * a[9] - a[8] * a[1] * a[7] + a[8] * a[3] * a[5];
            invOut[15] = a[0] * a[5] * a[10] - a[0] * a[6] * a[9] - a[4] * a[1] * a[10]
                + a[4] * a[2] * a[9] + a[8] * a[1] * a[6] - a[8] * a[2] * a[5];

            const float det = a[0] * invOut[0] + a[1] * invOut[4] + a[2] * invOut[8] + a[3] * invOut[12];
            if (det == 0.0f)
            {
                return identity();
            }

            const float invDet = 1.0f / det;
            for (float& value : inv.m)
            {
                value *= invDet;
            }

            return inv;
        }
    };

    inline Vec4 operator*(const Mat4& m, const Vec4& v)
    {
        Vec4 result;
        result.x = m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z + m(0, 3) * v.w;
        result.y = m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z + m(1, 3) * v.w;
        result.z = m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z + m(2, 3) * v.w;
        result.w = m(3, 0) * v.x + m(3, 1) * v.y + m(3, 2) * v.z + m(3, 3) * v.w;
        return result;
    }

    inline Mat4 operator*(const Mat4& a, const Mat4& b)
    {
        Mat4 result = {};
        for (size_t row = 0; row < 4; ++row)
        {
            for (size_t col = 0; col < 4; ++col)
            {
                float sum = 0.0f;
                for (size_t k = 0; k < 4; ++k)
                {
                    sum += a(row, k) * b(k, col);
                }
                result(row, col) = sum;
            }
        }
        return result;
    }
}
