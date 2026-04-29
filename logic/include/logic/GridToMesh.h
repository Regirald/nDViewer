#pragma once

#include "model/Mesh.h"
#include "core/Grid.h"
#include "core/BinomialCoefficient.h"
#include "GridLooper.h"


template <size_t Dim, typename T>
class GridToMesh
{
public:
	static Mesh<Dim> convert(const Grid<Dim>& grid);
	static void process(const typename Grid<Dim>::Index& index, const Grid<Dim>& grid, Mesh<Dim>& mesh);
};


template <size_t Dim, typename T>
inline Mesh<Dim> GridToMesh<Dim, T>::convert(const Grid<Dim>& grid)
{
    Mesh<Dim> mesh;
    Vec<Dim, int> size = grid.getSize();
    int nbPoints = grid.getNbPoints();
    mesh.vertices.resize(nbPoints);
    GridLooper<Dim>::loop(size, [&](typename Grid<Dim>::Index& index) {
        process(index, grid, mesh);
        });


    return mesh;
}

template <size_t Dim, typename T>
inline void GridToMesh<Dim, T>::process(const typename Grid<Dim>::Index& index,
    const Grid<Dim>& grid,
    Mesh<Dim>& mesh)
{
    const auto currentIdx = grid.flatten(index);
    mesh.vertices[currentIdx] = grid.getPosition(index);

    const auto& size = grid.getSize();

    std::vector<typename Grid<Dim>::Index> neighbors;
    neighbors.reserve(Dim);
    // add current to neighbor connections
    for (size_t d = 0; d < Dim; ++d)
    {
        typename Grid<Dim>::Index neighbor = index;
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