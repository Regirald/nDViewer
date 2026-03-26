#pragma once
#include <array>
#include "model/Mesh.h"
#include "core/Field.h"

class MarchingSquares {
public:
    using Vec2 = Vec<2, float>;

    struct Edge { int a, b; };

    static Vec2 interp(const Vec2& a, const Vec2& b, float va, float vb, float iso)
    {
        float t = (iso - va) / (vb - va);
        return a + t * (b - a);
    }

    static inline float cross(const Vec2& a, const Vec2& b)
    {
        return a[0] * b[1] - a[1] * b[0];
    }

    static Mesh<2> extract(const ScalarField<2>& field, float iso)
    {
        Mesh<2> mesh;

        int dimx = field.getSize()[0];
        int dimy = field.getSize()[1];

        // Local triangle edges
        static const Edge edges[3] = {
            {0, 1}, {1, 2}, {2, 0}
        };

        // Quad split into 2 triangles (index-based)
        static const int tris[2][3] = {
            {0, 1, 2},
            {0, 2, 3}
        };

        auto emitTriangle = [&](Vec2 a, Vec2 b, Vec2 c)
            {
                if (cross(b - a, c - a) < 0.0f)
                    std::swap(b, c);

                // Record starting index of this triangle's vertices
                int baseIndex = static_cast<int>(mesh.vertices.size());

                mesh.vertices.push_back(a);
                mesh.vertices.push_back(b);
                mesh.vertices.push_back(c);

                // Push back triangle indices (counter-clockwise order ensured above)
                mesh.indices.push_back(baseIndex);
                mesh.indices.push_back(baseIndex + 1);
                mesh.indices.push_back(baseIndex + 2);
            };

        auto emitEdge = [&](Vec2 a, Vec2 b)
            {
                // Record starting index of this triangle's vertices
                int baseIndex = static_cast<int>(mesh.vertices.size());

                mesh.vertices.push_back(a);
                mesh.vertices.push_back(b);

                // Push back triangle indices (counter-clockwise order ensured above)
                mesh.indices.push_back(baseIndex);
                mesh.indices.push_back(baseIndex + 1);
            };

        for (int y = 0; y < dimy - 1; ++y)
        {
            for (int x = 0; x < dimx - 1; ++x)
            {
                // Quad positions
                std::array<Vec2, 4> pos = {
                    Vec2{(float(x) / (dimx - 1)) * 2.0f - 1.0f,     (static_cast<float>(y) / (dimy - 1)) * 2.0f - 1.0f },     // 0
                    Vec2{(float(x + 1) / (dimx - 1)) * 2.0f - 1.0f, (static_cast<float>(y) / (dimy - 1)) * 2.0f - 1.0f },     // 1
                    Vec2{(float(x + 1) / (dimx - 1)) * 2.0f - 1.0f, (static_cast<float>(y + 1) / (dimy - 1)) * 2.0f - 1.0f}, // 2
                    Vec2{(float(x) / (dimx - 1)) * 2.0f - 1.0f,     (static_cast<float>(y + 1) / (dimy - 1)) * 2.0f - 1.0f}  // 3
                };
                
                // Quad values
                std::array<float, 4> val = {
                    field({x, y}),
                    field({x + 1, y}),
                    field({x + 1, y + 1}),
                    field({x, y + 1})
                };

                // Process both triangles generically
                for (int t = 0; t < 2; ++t)
                {
                    // Local triangle data
                    Vec2 triPos[3];
                    float triVal[3];

                    for (int i = 0; i < 3; ++i)
                    {
                        int idx = tris[t][i];
                        triPos[i] = pos[idx];
                        triVal[i] = val[idx];
                    }

                    // Polygon (max 4 vertices)
                    Vec2 poly[4];
                    int count = 0;

                    for (int e = 0; e < 3; ++e)
                    {
                        int i0 = edges[e].a;
                        int i1 = edges[e].b;

                        bool in0 = triVal[i0] > iso;
                        bool in1 = triVal[i1] > iso;

                        if (in0)
                            poly[count++] = triPos[i0];

                        if (in0 != in1)
                            poly[count++] = interp(
                                triPos[i0], triPos[i1],
                                triVal[i0], triVal[i1],
                                iso
                            );
                    }

                    // Fan triangulation
                    for (int i = 1; i + 1 < count; ++i)
                    {
                        emitTriangle(poly[0], poly[i], poly[i + 1]);
                    }
                }
            }
        }

        return mesh;
    }
};