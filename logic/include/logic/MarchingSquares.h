#pragma once
#include <array>
#include "model/Mesh.h"
#include "core/Field.h"
#include "core/Grid.h"

class MarchingSquares {
public:
    using Vec2 = Vec<2, float>;
    using Index = Vec<2, int>;

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

    static Mesh<2> extractContour(const ImplicitFunction<2>& impFunction,
        float iso,
        Grid<2> grid)
    {
        Mesh<2> mesh;

        int dimx = grid.m_resolution.size[0];
        int dimy = grid.m_resolution.size[1];

        auto index = [&](int x, int y) {
            return x + y * dimx;
            };


        // --- 2. Precompute scalar field ---
        std::vector<float> field(dimx * dimy);
        for (int y = 0; y < dimy; ++y)
            for (int x = 0; x < dimx; ++x)
                field[index(x, y)] = impFunction.evaluate({ 
                (float)x / (grid.m_resolution.size[0] - 1) * 2.f - 1.f, 
                (float)y / (grid.m_resolution.size[1] - 1) * 2.f - 1.f });

        // --- 3. Edge cache (critical: no duplicates) ---
        enum EdgeType { E0 = 0, E1 = 1, E2 = 2 };

        std::vector<int> edgeVertex[3];
        for (int e = 0; e < 3; ++e)
            edgeVertex[e].assign(dimx * dimy, -1);

        // --- 4. Interpolation ---
        auto interp = [&](const Vec2& p0, const Vec2& p1,
            float v0, float v1) -> Vec2 {
                float t = (iso - v0) / (v1 - v0);
                return p0 + t * (p1 - p0);
            };

        // --- 5. Edge vertex creation (GLOBAL + UNIQUE) ---
        auto getEdgeVertex = [&](int xa, int ya, int xb, int yb)->int
            {
                EdgeType e;
                int x, y;

                if (ya == yb) // horizontal
                {
                    e = E0;
                    x = std::min(xa, xb);
                    y = ya;
                }
                else if (xa == xb) // vertical-ish
                {
                    e = E2;
                    x = xa;
                    y = std::min(ya, yb);
                }
                else // diagonal
                {
                    e = E1;
                    x = (ya < yb) ? xb : xa;
                    y = std::min(ya, yb);
                }

                int idx = index(x, y);
                int& slot = edgeVertex[e][idx];

                if (slot != -1)
                    return slot;

                Vec2 p0 = grid.getPosition(Index({ xa, ya }));
                Vec2 p1 = grid.getPosition(Index({ xb, yb }));

                float v0 = field[index(xa, ya)];
                float v1 = field[index(xb, yb)];

                Vec2 p = interp(p0, p1, v0, v1);

                int vid = (int)mesh.vertices.size();
                mesh.vertices.push_back(p);

                slot = vid;
                return vid;
            };

        // --- 6. Process triangle (SEGMENT extraction) ---
        auto processTriangle = [&](int x0, int y0,
            int x1, int y1,
            int x2, int y2)
            {
                float v0 = field[index(x0, y0)];
                float v1 = field[index(x1, y1)];
                float v2 = field[index(x2, y2)];

                bool in0 = v0 > iso;
                bool in1 = v1 > iso;
                bool in2 = v2 > iso;

                int vids[3];
                int count = 0;

                if (in0 != in1)
                    vids[count++] = getEdgeVertex(x0, y0, x1, y1);

                if (in1 != in2)
                    vids[count++] = getEdgeVertex(x1, y1, x2, y2);

                if (in2 != in0)
                    vids[count++] = getEdgeVertex(x2, y2, x0, y0);

                // --- VALID CASE ---
                if (count == 2)
                {
                    mesh.indices.push_back(vids[0]);
                    mesh.indices.push_back(vids[1]);
                }

                // ignore count == 0 or degenerate cases
            };

        // --- 7. Traverse grid ---
        for (int y = 0; y < dimy - 1; ++y)
        {
            for (int x = 0; x < dimx - 1; ++x)
            {
                if ((y & 1) == 0)
                {
                    processTriangle(x, y, x + 1, y, x, y + 1);
                    processTriangle(x + 1, y, x + 1, y + 1, x, y + 1);
                }
                else
                {
                    processTriangle(x, y, x + 1, y, x + 1, y + 1);
                    processTriangle(x, y, x + 1, y + 1, x, y + 1);
                }
            }
        }

        return mesh;
    }
};