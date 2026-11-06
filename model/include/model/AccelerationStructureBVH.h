#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

#include "model/GeometryFromTopology.h"

template <size_t Dim>
class AccelerationStructureBVH
{
public:

    using Scalar = float;
    using Vector = Vec<Dim, Scalar>;

    struct AABB
    {
        Vector min;
        Vector max;
    };

    struct Primitive
    {
        uint32_t index;
        AABB bounds;
        Vector centroid;
    };

    struct Node
    {
        AABB bounds;

        uint32_t leftChild = UINT32_MAX;
        uint32_t rightChild = UINT32_MAX;

        uint32_t firstPrimitive = 0;
        uint32_t primitiveCount = 0;

        bool isLeaf() const
        {
            return primitiveCount > 0;
        }
    };

    explicit AccelerationStructureBVH(
        const PolytopeGeometry<Dim>* polytope,
        Scalar primitivePadding = 0.0f)
        : m_polytope(polytope), m_primitivePadding(primitivePadding)
    {
    }

    void build()
    {
        m_nodes.clear();
        m_primitives.clear();

        if (!m_polytope)
            return;

        buildPrimitives();

        if (m_primitives.empty())
            return;

        m_nodes.reserve(m_primitives.size() * 2);

        buildRecursive(
            0,
            static_cast<uint32_t>(m_primitives.size()));
    }

    const std::vector<Node>& nodes() const
    {
        return m_nodes;
    }

    const std::vector<Primitive>& primitives() const
    {
        return m_primitives;
    }

    bool empty() const
    {
        return m_nodes.empty();
    }

    uint32_t rootIndex() const
    {
        return 0;
    }

private:

    const PolytopeGeometry<Dim>* m_polytope = nullptr;
    Scalar m_primitivePadding = 0.0f;

    std::vector<Node> m_nodes;
    std::vector<Primitive> m_primitives;

    static constexpr uint32_t MaxLeafSize = 4;

private:

    // ---------------------------------------------------------
    // Primitive generation
    // ---------------------------------------------------------

    void buildPrimitives()
    {
        const auto& vertices = m_polytope->vertices;

        m_primitives.reserve(vertices.size());

        for (uint32_t index = 0;
            index < vertices.size();
            ++index)
        {
            const auto& vertex = vertices[index];

            Primitive primitive;
            primitive.index = index;

            initializeAABB(primitive.bounds);

            expandAABB(primitive.bounds, vertex);

            primitive.centroid = vertex;

            for (size_t d = 0; d < Dim; ++d)
            {
                primitive.bounds.min[d] -= m_primitivePadding;
                primitive.bounds.max[d] += m_primitivePadding;
            }

            m_primitives.push_back(primitive);
        }
    }

    // ---------------------------------------------------------
    // AABB operations
    // ---------------------------------------------------------

    static void initializeAABB(AABB& bounds)
    {
        for (size_t d = 0; d < Dim; ++d)
        {
            bounds.min[d] =
                std::numeric_limits<Scalar>::max();

            bounds.max[d] =
                std::numeric_limits<Scalar>::lowest();
        }
    }

    static void expandAABB(
        AABB& bounds,
        const Vector& point)
    {
        for (size_t d = 0; d < Dim; ++d)
        {
            bounds.min[d] =
                std::min(bounds.min[d], point[d]);

            bounds.max[d] =
                std::max(bounds.max[d], point[d]);
        }
    }

    AABB calculateBounds(
        uint32_t first,
        uint32_t count) const
    {
        AABB bounds;
        initializeAABB(bounds);

        for (uint32_t i = first; i < first + count; ++i)
        {
            const AABB& primitiveBounds =
                m_primitives[i].bounds;

            for (size_t d = 0; d < Dim; ++d)
            {
                bounds.min[d] = std::min(
                    bounds.min[d],
                    primitiveBounds.min[d]);

                bounds.max[d] = std::max(
                    bounds.max[d],
                    primitiveBounds.max[d]);
            }
        }

        return bounds;
    }

    // ---------------------------------------------------------
    // Split axis selection
    // ---------------------------------------------------------

    size_t findLargestAxis(
        uint32_t first,
        uint32_t count) const
    {
        Vector minCentroid;
        Vector maxCentroid;

        for (size_t d = 0; d < Dim; ++d)
        {
            minCentroid[d] =
                std::numeric_limits<Scalar>::max();

            maxCentroid[d] =
                std::numeric_limits<Scalar>::lowest();
        }

        for (uint32_t i = first; i < first + count; ++i)
        {
            const Vector& centroid =
                m_primitives[i].centroid;

            for (size_t d = 0; d < Dim; ++d)
            {
                minCentroid[d] = std::min(
                    minCentroid[d], centroid[d]);

                maxCentroid[d] = std::max(
                    maxCentroid[d], centroid[d]);
            }
        }

        size_t bestAxis = 0;
        Scalar bestExtent = 0;

        for (size_t d = 0; d < Dim; ++d)
        {
            Scalar extent =
                maxCentroid[d] - minCentroid[d];

            if (extent > bestExtent)
            {
                bestExtent = extent;
                bestAxis = d;
            }
        }

        return bestAxis;
    }

    // ---------------------------------------------------------
    // Recursive BVH construction
    // ---------------------------------------------------------

    uint32_t buildRecursive(
        uint32_t first,
        uint32_t count)
    {
        const uint32_t nodeIndex =
            static_cast<uint32_t>(m_nodes.size());

        m_nodes.emplace_back();

        m_nodes[nodeIndex].bounds =
            calculateBounds(first, count);

        if (count <= MaxLeafSize)
        {
            m_nodes[nodeIndex].firstPrimitive = first;
            m_nodes[nodeIndex].primitiveCount = count;

            return nodeIndex;
        }

        const size_t axis =
            findLargestAxis(first, count);

        const uint32_t middle =
            first + count / 2;

        std::nth_element(
            m_primitives.begin() + first,
            m_primitives.begin() + middle,
            m_primitives.begin() + first + count,
            [axis](const Primitive& a, const Primitive& b)
            {
                return a.centroid[axis] < b.centroid[axis];
            });

        const uint32_t left =
            buildRecursive(first, middle - first);

        const uint32_t right =
            buildRecursive(middle, first + count - middle);

        m_nodes[nodeIndex].leftChild = left;
        m_nodes[nodeIndex].rightChild = right;

        return nodeIndex;
    }
};



template class AccelerationStructureBVH<2>;
template class AccelerationStructureBVH<3>;
template class AccelerationStructureBVH<4>;
