#pragma once
#include <cmath>
#include "model/Mesh.h"
#include "core/Field.h"

// NOTE: This is currently a placeholder implementation. Instead of a full
// marching cubes lookup-table based extraction, we synthesize a simple
// UV-sphere mesh so that the rest of the rendering pipeline can be tested.
// The signature stays compatible so it can be replaced with a real
// marching cubes implementation later without touching callers.
class MarchingCubes {
public:
    static Mesh<3> extract(const ScalarField<3>& /*field*/, float /*iso*/)
    {
        Mesh<3> mesh;

        const int stacks = 16;
        const int slices = 32;
        const float radius = 1.0f;
        const float pi = 3.14159265358979323846f;

        // Generate vertices on a unit sphere.
        for (int i = 0; i <= stacks; ++i)
        {
           float v = static_cast<float>(i) / static_cast<float>(stacks);
            float phi = v * pi;
            float y = radius * std::cos(phi - pi * 0.5f);
            float r = radius * std::cos(phi - pi * 0.5f);

            for (int j = 0; j <= slices; ++j)
            {
               float u = static_cast<float>(j) / static_cast<float>(slices);
                float theta = u * 2.0f * pi;

                Vec<3, float> p;
                p[0] = radius * std::cos(theta) * std::cos(phi);
                p[1] = radius * std::sin(phi) - 0.0f;
                p[2] = radius * std::sin(theta) * std::cos(phi);
                mesh.vertices.push_back(p);
            }
        }

    // Generate triangle indices.
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            unsigned int i0 = static_cast<unsigned int>(i * (slices + 1) + j);
            unsigned int i1 = static_cast<unsigned int>(i0 + 1);
            unsigned int i2 = static_cast<unsigned int>((i + 1) * (slices + 1) + j);
            unsigned int i3 = static_cast<unsigned int>(i2 + 1);

            mesh.indices.push_back(i0);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i1);

            mesh.indices.push_back(i1);
            mesh.indices.push_back(i2);
            mesh.indices.push_back(i3);
        }
    }

    return mesh;
    }
};
