#pragma once
#include <cmath>
#include "model/Mesh.h"
#include "core/Field.h"


class MarchingCubes {
public:
    using Vec3 = Vec<3, float>;

    static Vec3 interp(const Vec3& a, const Vec3& b, float va, float vb, float iso)
    {
        float t = (iso - va) / (vb - va);
        return a + t * (b - a);
    }

    static Mesh<3> extractContour(const ImplicitFunction<3>& impFunction,
        float iso,
        Grid<3, float> grid)
    {
        Mesh<3> mesh;

        int dimx = grid.m_resolution.size[0];
        int dimy = grid.m_resolution.size[1];
        int dimz = grid.m_resolution.size[2];

        auto index = [&](int x, int y, int z) {
            return x + dimx * (y + dimy * z);
            };

        // --- 1. Precompute scalar field ---
        std::vector<float> field(dimx * dimy * dimz);

        for (int z = 0; z < dimz; ++z)
            for (int y = 0; y < dimy; ++y)
                for (int x = 0; x < dimx; ++x)
                {
                    field[index(x, y, z)] = impFunction.evaluate({
                        (float)x / (dimx - 1) * 2.f - 1.f,
                        (float)y / (dimy - 1) * 2.f - 1.f,
                        (float)z / (dimz - 1) * 2.f - 1.f
                        });
                }

        // --- 2. Edge cache ---
        enum EdgeType { EX = 0, EY = 1, EZ = 2 };

        std::vector<int> edgeVertex[3];
        for (int e = 0;e < 3;++e)
            edgeVertex[e].assign(dimx * dimy * dimz, -1);

        auto getEdgeVertex = [&](int xa, int ya, int za,
            int xb, int yb, int zb)
            {
                EdgeType e;
                int x, y, z;

                if (ya == yb && za == zb) { e = EX; x = std::min(xa, xb); y = ya; z = za; }
                else if (xa == xb && za == zb) { e = EY; x = xa; y = std::min(ya, yb); z = za; }
                else { e = EZ; x = xa; y = ya; z = std::min(za, zb); }

                int idx = index(x, y, z);
                int& slot = edgeVertex[e][idx];

                if (slot != -1) return slot;

                Vec3 p0 = grid.getPosition({ xa,ya,za });
                Vec3 p1 = grid.getPosition({ xb,yb,zb });

                float v0 = field[index(xa, ya, za)];
                float v1 = field[index(xb, yb, zb)];

                Vec3 p = interp(p0, p1, v0, v1, iso);

                int vid = (int)mesh.vertices.size();
                mesh.vertices.push_back(p);

                slot = vid;
                return vid;
            };

        // --- 3. Process tetrahedron ---
        auto processTet = [&](int v[4][3])
            {
                float val[4];
                bool inside[4];

                for (int i = 0;i < 4;++i)
                {
                    val[i] = field[index(v[i][0], v[i][1], v[i][2])];
                    inside[i] = val[i] > iso;
                }

                int vids[6];
                int count = 0;

                auto edge = [&](int a, int b)
                    {
                        vids[count++] = getEdgeVertex(
                            v[a][0], v[a][1], v[a][2],
                            v[b][0], v[b][1], v[b][2]);
                    };

                if (inside[0] != inside[1]) edge(0, 1);
                if (inside[0] != inside[2]) edge(0, 2);
                if (inside[0] != inside[3]) edge(0, 3);
                if (inside[1] != inside[2]) edge(1, 2);
                if (inside[1] != inside[3]) edge(1, 3);
                if (inside[2] != inside[3]) edge(2, 3);

                if (count == 3)
                {
                    mesh.indices.push_back(vids[0]);
                    mesh.indices.push_back(vids[1]);
                    mesh.indices.push_back(vids[2]);
                }
                else if (count == 4)
                {
                    mesh.indices.push_back(vids[0]);
                    mesh.indices.push_back(vids[1]);
                    mesh.indices.push_back(vids[2]);

                    mesh.indices.push_back(vids[0]);
                    mesh.indices.push_back(vids[2]);
                    mesh.indices.push_back(vids[3]);
                }
            };

        // --- 4. Cube traversal ---
        const int tet[6][4] =
        {
            {0,5,1,6},
            {0,1,2,6},
            {0,2,3,6},
            {0,3,7,6},
            {0,7,4,6},
            {0,4,5,6}
        };

        int cubeVerts[8][3];

        for (int z = 0;z < dimz - 1;++z)
            for (int y = 0;y < dimy - 1;++y)
                for (int x = 0;x < dimx - 1;++x)
                {
                    cubeVerts[0][0] = x;   cubeVerts[0][1] = y;   cubeVerts[0][2] = z;
                    cubeVerts[1][0] = x + 1; cubeVerts[1][1] = y;   cubeVerts[1][2] = z;
                    cubeVerts[2][0] = x + 1; cubeVerts[2][1] = y + 1; cubeVerts[2][2] = z;
                    cubeVerts[3][0] = x;   cubeVerts[3][1] = y + 1; cubeVerts[3][2] = z;
                    cubeVerts[4][0] = x;   cubeVerts[4][1] = y;   cubeVerts[4][2] = z + 1;
                    cubeVerts[5][0] = x + 1; cubeVerts[5][1] = y;   cubeVerts[5][2] = z + 1;
                    cubeVerts[6][0] = x + 1; cubeVerts[6][1] = y + 1; cubeVerts[6][2] = z + 1;
                    cubeVerts[7][0] = x;   cubeVerts[7][1] = y + 1; cubeVerts[7][2] = z + 1;

                    for (int t = 0;t < 6;++t)
                    {
                        int v[4][3];
                        for (int i = 0;i < 4;++i)
                        {
                            int id = tet[t][i];
                            v[i][0] = cubeVerts[id][0];
                            v[i][1] = cubeVerts[id][1];
                            v[i][2] = cubeVerts[id][2];
                        }

                        processTet(v);
                    }
                }

        return mesh;
    }
};
