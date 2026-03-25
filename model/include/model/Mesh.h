#pragma once
#include <vector>
#include "core/Vec.h"

// Simple triangle mesh holding positions only. For Dim != 3 this is just
// a generic container; for Dim == 3 we will interpret it as a 3D surface.
template <size_t Dim>
struct Mesh {
    std::vector<Vec<Dim, float>> vertices;   // vertex positions
    std::vector<unsigned int> indices;       // triangle index buffer (3 per tri)

    void clear()
    {
        vertices.clear();
        indices.clear();
    }

    bool empty() const
    {
        return vertices.empty() || indices.empty();
    }
};
