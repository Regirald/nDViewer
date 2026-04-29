#pragma once
#include <cmath>
#include <vector>
#include <array>
#include <queue>
#include <functional>
#include <algorithm>
#include "core/Field.h"
#include "model/Mesh.h"
#include "model/ImplicitFunction.h"
#include "GridLooper.h"

template <size_t Dim>
class MarchingHyperCubes
{
public:
    using Index = Vec<Dim, int>;
    using Point = Vec<Dim, float>;
    using Simplex = std::array<Index, Dim + 1>;
    using PatchSimplex = std::array<Point, Dim>;

    struct EdgeIntersection
    {
        size_t v0;
        size_t v1;
        Point position;
    };
    static std::vector<Simplex> generateFreudenthalSimplices();
    static std::vector<EdgeIntersection> processSimplex(const Simplex& simplex, ScalarField<Dim>& field, double isoValue);
    static std::vector<PatchSimplex> triangulateIntersectionPatchConvexHull(const std::vector<EdgeIntersection>& intersections);
    static Mesh<Dim> extractIsosurface(const ImplicitFunction<Dim>& impFunction, float iso, Grid<Dim> grid);

    static Point interpolate(const Point& a, const Point& b, float va, float vb, float iso)
    {
        float t = (iso - va) / (vb - va);
        return a + t * (b - a);
    }
};

template <size_t Dim>
Mesh<Dim> MarchingHyperCubes<Dim>::extractIsosurface(const ImplicitFunction<Dim>& impFunction,
	float iso,
	Grid<Dim> grid) 
{
	Mesh<Dim> mesh;

	// --- 1. Precompute scalar field ---
	ScalarField<Dim> field(grid.m_resolution, grid.m_basis);
	GridLooper<Dim, Dim>::loop(grid.getSize(), [&](const Index& index) {
		field[index] = impFunction.evaluate(grid.getPosition(index));
		});



	// --- 3. Process hypercube ---
    int nbProcessedMax = 100;
    std::vector<typename MarchingHyperCubes<Dim>::Simplex> simplices = generateFreudenthalSimplices();
	auto processHyperCube = [&iso, &mesh,&nbProcessedMax, &field, simplices](const Index& idxCube) //simplices passed by copy!
		{
            std::ostringstream oss;
            oss << idxCube;
            if (nbProcessedMax == 0)
                return;
            nbProcessedMax--;
            qDebug() << QString::fromStdString(oss.str());
			for (auto simplice : simplices)
            {
                for (size_t i = 0; i < Dim + 1; ++i)
                {
					for (size_t d = 0; d < Dim; ++d)
					    simplice[i][d] += idxCube[d]; // translate simplex to current cube position
                }
                std::vector<typename MarchingHyperCubes<Dim>::EdgeIntersection> intersections = processSimplex(simplice, field, iso);
                std::vector<PatchSimplex> patchs = triangulateIntersectionPatchConvexHull(intersections);
				for (auto patch : patchs)
                {
                    for (const auto& vertex : patch) {
                            mesh.vertices.push_back(field.getPosition(vertex));
                            mesh.indices.push_back(mesh.vertices.size() - 1);
                    }
                }
            }

		};
			
	Index maxIndex = grid.getSize() - Index(1); // max cube index is one less than grid size
    GridLooper<Dim, Dim>::loop(maxIndex, [&](const Index& index) {
        processHyperCube(index);
        });

	return mesh;
}



template <size_t Dim>
std::vector<typename MarchingHyperCubes<Dim>::Simplex> MarchingHyperCubes<Dim>::generateFreudenthalSimplices()
{
    std::vector<typename MarchingHyperCubes<Dim>::Simplex> simplices;

	// permutation of axes: [0,1,2,...,Dim-1]
	std::array<size_t, Dim> perm;
	for (size_t i = 0; i < Dim; ++i)
		perm[i] = i;

	do
	{
        typename MarchingHyperCubes<Dim>::Simplex simplex{};

		// v0 = origin already zero-initialized

        typename MarchingHyperCubes<Dim>::Index current{};
        simplex[0] = current;

		// progressively add unit vectors
		for (size_t k = 0; k < Dim; ++k)
		{
			current[perm[k]] = 1;
			simplex[k + 1] = current;
		}

		simplices.push_back(simplex);

	} while (std::next_permutation(perm.begin(), perm.end()));

	return simplices;
}



template <size_t Dim>
std::vector<typename MarchingHyperCubes<Dim>::EdgeIntersection>
MarchingHyperCubes<Dim>::processSimplex(const typename MarchingHyperCubes<Dim>::Simplex& simplex, ScalarField<Dim>& field, double isoValue)
{
    std::vector<EdgeIntersection> intersections;

    // scalar values at simplex vertices
    std::array<double, Dim + 1> values{};

    for (size_t i = 0; i < Dim + 1; ++i)
        values[i] = field[simplex[i]];

    // test all simplex edges
    for (size_t i = 0; i < Dim + 1; ++i)
    {
        for (size_t j = i + 1; j < Dim + 1; ++j)
        {
            bool side0 = values[i] < isoValue;
            bool side1 = values[j] < isoValue;

            // edge crosses isosurface
            if (side0 != side1)
            {
                EdgeIntersection inter;

                inter.v0 = i;
                inter.v1 = j;

                // Convert integer index vertices to float points for interpolation
                Point p0; Point p1;
                for (size_t d = 0; d < Dim; ++d) {
                    p0[d] = static_cast<float>(simplex[i][d]);
                    p1[d] = static_cast<float>(simplex[j][d]);
                }

                inter.position = interpolate(
                    p0, 
                    p1,
                    values[i],
                    values[j],
                    isoValue);

                intersections.push_back(inter);
            }
        }
    }

    return intersections;
}




// -----------------------------------------------------------------------------
// Exact triangulation of the convex intersection patch of a simplex.
// This is the staircase triangulation of the product of simplices
// Δ^(p-1) × Δ^(q-1), where p and q are the two sign groups of the simplex
// vertices.
// -----------------------------------------------------------------------------
template <size_t Dim>
std::vector<typename MarchingHyperCubes<Dim>::PatchSimplex>
MarchingHyperCubes<Dim>::triangulateIntersectionPatchConvexHull(
    const std::vector<typename MarchingHyperCubes<Dim>::EdgeIntersection>& intersections)
{
    std::vector<typename MarchingHyperCubes<Dim>::PatchSimplex> result;

    if (intersections.empty())
        return result;

    // There are Dim+1 simplex vertices total.
    // Build the graph of crossed edges.
    std::array<std::vector<size_t>, Dim + 1> incident;
    for (const auto& e : intersections)
    {
        if (e.v0 >= Dim + 1 || e.v1 >= Dim + 1 || e.v0 == e.v1)
            return {};

        incident[e.v0].push_back(e.v1);
        incident[e.v1].push_back(e.v0);
    }

    // 2-color the graph to recover the two sign groups.
    // For a generic simplex slice, the crossed-edge graph is complete bipartite.
    std::array<int, Dim + 1> color;
    color.fill(-1);

    std::queue<size_t> q;
    color[intersections.front().v0] = 0;
    q.push(intersections.front().v0);

    while (!q.empty())
    {
        const size_t u = q.front();
        q.pop();

        for (size_t v : incident[u])
        {
            if (color[v] == -1)
            {
                color[v] = 1 - color[u];
                q.push(v);
            }
            else if (color[v] == color[u])
            {
                // Not bipartite => degenerate or invalid input.
                return {};
            }
        }
    }

    // Every vertex incident to a crossing edge must have been colored.
    for (size_t v = 0; v < Dim + 1; ++v)
    {
        if (!incident[v].empty() && color[v] == -1)
            return {};
    }

    // Build the two vertex groups.
    std::vector<size_t> groupA;
    std::vector<size_t> groupB;

    for (size_t v = 0; v < Dim + 1; ++v)
    {
        if (color[v] == 0)
            groupA.push_back(v);
        else if (color[v] == 1)
            groupB.push_back(v);
    }

    if (groupA.empty() || groupB.empty())
        return {};

    std::sort(groupA.begin(), groupA.end());
    std::sort(groupB.begin(), groupB.end());

    const size_t p = groupA.size();
    const size_t qSize = groupB.size();

    // For a simplex slice, every pair (A_i, B_j) corresponds to one
    // intersection point on the edge (A_i, B_j).
    std::array<int, Dim + 1> indexInA;
    std::array<int, Dim + 1> indexInB;
    indexInA.fill(-1);
    indexInB.fill(-1);

    for (size_t i = 0; i < p; ++i)
        indexInA[groupA[i]] = static_cast<int>(i);

    for (size_t j = 0; j < qSize; ++j)
        indexInB[groupB[j]] = static_cast<int>(j);

    std::vector<std::vector<typename MarchingHyperCubes<Dim>::Point>> cross(
        p, std::vector<typename MarchingHyperCubes<Dim>::Point>(qSize));

    std::vector<std::vector<bool>> have(
        p, std::vector<bool>(qSize, false));

    for (const auto& e : intersections)
    {
        const bool a0 = (color[e.v0] == 0);
        const bool a1 = (color[e.v1] == 0);

        // Must be a crossing edge, so endpoints must have opposite colors.
        if (a0 == a1)
            return {};

        size_t i = 0;
        size_t j = 0;

        if (a0)
        {
            i = static_cast<size_t>(indexInA[e.v0]);
            j = static_cast<size_t>(indexInB[e.v1]);
        }
        else
        {
            i = static_cast<size_t>(indexInA[e.v1]);
            j = static_cast<size_t>(indexInB[e.v0]);
        }

        if (i >= p || j >= qSize)
            return {};

        cross[i][j] = e.position;
        have[i][j] = true;
    }

    // Generic simplex slice should have all p*q crossing vertices.
    for (size_t i = 0; i < p; ++i)
        for (size_t j = 0; j < qSize; ++j)
            if (!have[i][j])
                return {};

    // Staircase triangulation:
    // each monotone path from (0,0) to (p-1,qSize-1) yields one simplex.
    // The path visits exactly (p-1) + (qSize-1) + 1 = Dim vertices.
    typename MarchingHyperCubes<Dim>::PatchSimplex current{};

    std::function<void(size_t, size_t, size_t)> dfs =
        [&](size_t i, size_t j, size_t depth)
        {
            current[depth] = cross[i][j];

            if (i == p - 1 && j == qSize - 1)
            {
                if (depth + 1 == Dim)
                    result.push_back(current);
                return;
            }

            if (i < p - 1)
                dfs(i + 1, j, depth + 1);

            if (j < qSize - 1)
                dfs(i, j + 1, depth + 1);
        };

    dfs(0, 0, 0);

    return result;
}