#pragma once

// WythoffTopology.cpp
#include "CoxeterPresentation.h"
#include "ToddCoxeter.h"

#include <vector>
#include <queue>
#include <set>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

namespace Coxeter
{

    struct Feature
    {
        int id = -1;
        int rank = -1;                 // 0=vertex, 1=edge, 2=face, 3=cell, ...
        int coset = -1;                // row in the corresponding coset table
        int canonicalParent = -1;      // one chosen parent in the previous rank
        std::vector<uint16_t> representativeWord;

        // Hierarchical links only, not full boundary extraction
        std::vector<int> children;
    };

    struct Polytope
    {
        int dimension = 0;

        std::vector<Feature> vertices;
        std::vector<Feature> edges;
        std::vector<Feature> faces;
        std::vector<Feature> cells;

        // ranks >= 4
        std::vector<std::vector<Feature>> higherRanks;

        // actual 1-skeleton for rendering
        std::vector<std::pair<int, int>> edgeVertexPairs;
    };

    struct FeatureLayer
    {
        int rank = -1;
        Subgroup stabilizer;
        CosetTable table;
        std::vector<std::vector<uint16_t>> representatives;
        std::vector<Feature> features;
    };

    static Subgroup makeFeatureStabilizer(int coxeterRank, int featureRank)
    {
        // W/H_k with H_k = < s_{k+1}, ..., s_{r-1} >
        Subgroup H;

        for (int g = featureRank + 1; g < coxeterRank; ++g)
            H.generators.push_back(g);

        return H;
    }

    static std::vector<Relator> buildCanonicalRelators(const Presentation& P)
    {
        return buildRelators(P);
    }

    static std::vector<std::vector<uint16_t>> buildRepresentativeWords(const CosetTable& table)
    {
        const int cosetCount = static_cast<int>(table.table.size());
        const int genCount = table.generators;

        std::vector<std::vector<uint16_t>> reps(cosetCount);
        std::vector<bool> visited(cosetCount, false);
        std::queue<int> q;

        if (cosetCount == 0)
            return reps;

        visited[0] = true;
        q.push(0);

        while (!q.empty())
        {
            const int c = q.front();
            q.pop();

            for (int g = 0; g < genCount; ++g)
            {
                int n = table.table[c][g];
                if (n < 0 || n >= cosetCount)
                    continue;

                if (!visited[n])
                {
                    visited[n] = true;
                    reps[n] = reps[c];
                    reps[n].push_back(static_cast<uint16_t>(g));
                    q.push(n);
                }
            }
        }

        return reps;
    }

    static int followWord(const CosetTable& table, const std::vector<uint16_t>& word)
    {
        if (table.table.empty())
            return -1;

        int c = 0;

        for (uint16_t g : word)
        {
            if (g >= static_cast<uint16_t>(table.generators))
                throw std::out_of_range("followWord: generator out of range");

            c = table.table[c][g];

            if (c < 0)
                return -1;
        }

        return c;
    }

    static FeatureLayer buildFeatureLayer(
        const Presentation& P,
        const std::vector<Relator>& relators,
        int featureRank)
    {
        FeatureLayer layer;
        layer.rank = featureRank;
        layer.stabilizer = makeFeatureStabilizer(P.rank, featureRank);

        layer.table =
            enumerateCosets(
                P,
                relators,
                layer.stabilizer);

        layer.representatives =
            buildRepresentativeWords(layer.table);

        const int count = static_cast<int>(layer.table.table.size());
        layer.features.resize(count);

        for (int i = 0; i < count; ++i)
        {
            Feature f;
            f.id = i;
            f.rank = featureRank;
            f.coset = i;
            f.representativeWord = layer.representatives[i];
            layer.features[i] = std::move(f);
        }

        return layer;
    }

    static void linkParentChild(
        FeatureLayer& parent,
        FeatureLayer& child)
    {
        // canonical parent = evaluate the child's representative word in the parent layer
        for (Feature& f : child.features)
        {
            const int parentCoset =
                followWord(parent.table, f.representativeWord);

            f.canonicalParent = parentCoset;

            if (parentCoset >= 0 && parentCoset < static_cast<int>(parent.features.size()))
                parent.features[parentCoset].children.push_back(f.id);
        }

        // dedupe child lists
        for (Feature& p : parent.features)
        {
            std::sort(p.children.begin(), p.children.end());
            p.children.erase(std::unique(p.children.begin(), p.children.end()), p.children.end());
        }
    }

    static std::vector<std::pair<int, int>> buildVertexEdgesFromVertexLayer(const FeatureLayer& vertexLayer)
    {
        std::set<std::pair<int, int>> uniqueEdges;
        std::vector<std::pair<int, int>> edges;

        if (vertexLayer.table.table.empty())
            return edges;

        // Standard Wythoff choice: generator 0 moves along an edge of the 1-skeleton.
        const int edgeGenerator = 0;

        for (int v = 0; v < static_cast<int>(vertexLayer.table.table.size()); ++v)
        {
            const int n = vertexLayer.table.table[v][edgeGenerator];
            if (n < 0)
                continue;

            int a = v;
            int b = n;

            if (a == b)
                continue;

            if (a > b)
                std::swap(a, b);

            if (uniqueEdges.insert({ a, b }).second)
                edges.push_back({ a, b });
        }

        return edges;
    }

    static std::vector<std::vector<Feature>> buildHigherRanks(
        const std::vector<FeatureLayer>& layers,
        int fromRank)
    {
        std::vector<std::vector<Feature>> higher;
        for (int r = fromRank; r < static_cast<int>(layers.size()); ++r)
            higher.push_back(layers[r].features);
        return higher;
    }

    Polytope buildAbstractPolytope(const Presentation& P)
    {
        const std::vector<Relator> relators =
            buildCanonicalRelators(P);

        const int maxFeatureRank = P.rank - 1;

        std::vector<FeatureLayer> layers;
        layers.reserve(P.rank);

        for (int featureRank = 0; featureRank <= maxFeatureRank; ++featureRank)
            layers.push_back(buildFeatureLayer(P, relators, featureRank));

        for (int r = 1; r < static_cast<int>(layers.size()); ++r)
            linkParentChild(layers[r - 1], layers[r]);

        Polytope poly;
        poly.dimension = P.rank;

        if (layers.size() > 0) poly.vertices = layers[0].features;
        if (layers.size() > 1) poly.edges = layers[1].features;
        if (layers.size() > 2) poly.faces = layers[2].features;
        if (layers.size() > 3) poly.cells = layers[3].features;

        if (layers.size() > 4)
        {
            poly.higherRanks =
                buildHigherRanks(layers, 4);
        }

        poly.edgeVertexPairs =
            buildVertexEdgesFromVertexLayer(layers[0]);

        return poly;
    }

    Polytope buildAbstractPolytope(const std::vector<int>& schlafli)
    {
        Presentation P = buildPresentation(schlafli);
        return buildAbstractPolytope(P);
    }

} // namespace Coxeter