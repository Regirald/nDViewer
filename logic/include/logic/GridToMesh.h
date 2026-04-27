#pragma once

#include "model/Mesh.h"
#include "core/Grid.h"
#include "core/BinomialCoefficient.h"
#include "GridLooper.h"


template <size_t Dim, typename T>
class GridToMesh
{
public:
	static Mesh<Dim> convert(const Grid<Dim, T>& grid);
	static void process(const Vec<Dim, int>& index, const Grid<Dim, T>& grid, Mesh<Dim>& mesh);
};


template <size_t Dim, typename T>
inline Mesh<Dim> GridToMesh<Dim, T>::convert(const Grid<Dim, T>& grid)
{
    Mesh<Dim> mesh;
    Vec<Dim, int> size = grid.getSize();
    int nbPoints = grid.getNbPoints();
    mesh.vertices.resize(nbPoints);
    GridLooper<Dim>::loop(size, [&](Vec<Dim, int>& index) {
        process(index, grid, mesh);
        });


    return mesh;
}

template <size_t Dim, typename T>
inline void GridToMesh<Dim, T>::process(const Vec<Dim, int>& index,
    const Grid<Dim, T>& grid,
    Mesh<Dim>& mesh)
{
    const auto currentIdx = grid.flatten(index);
    mesh.vertices[currentIdx] = grid.getPosition(index);

    const auto& size = grid.getSize();

    std::vector<Vec<Dim, int>> neighbors;
    neighbors.reserve(Dim);
    // add current to neighbor connections
    for (size_t d = 0; d < Dim; ++d)
    {
        Vec<Dim, int> neighbor = index;
        neighbor[d] += 1;

        if (neighbor[d] < size[d])
        {
            mesh.indices.push_back(currentIdx);
            mesh.indices.push_back(grid.flatten(neighbor));
            neighbors.push_back(neighbor);
        }
    }
        // add neighbor to neighbor connections
    //if (neighbors.size() >= 2)
    //{

    //    std::vector<std::vector<int>> combos = generateCombinations(static_cast<int>(neighbors.size()), 2);
    //    for (const auto& combo : combos) {
    //        mesh.indices.push_back(grid.flatten(neighbors[combo[1]]));
    //        mesh.indices.push_back(grid.flatten(neighbors[combo[0]]));
    //    }
    //}
}