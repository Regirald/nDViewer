#pragma once

#include "model/Mesh.h"
#include "core/Grid.h"

template <size_t Dim, typename T>
class GridToMesh
{
public:
	static Mesh<Dim> convert(const Grid<Dim, T>& grid);
	static void process(const Vec<Dim, int>& index, const Grid<Dim, T>& grid, Mesh<Dim>& mesh);
};


template <size_t Dim, size_t CurrentDim>
struct GridLooper {
    template <typename Func>
    static void loop(const Vec<Dim, int>& size,
        Vec<Dim, int>& index,
        Func&& f)
    {
        for (index[CurrentDim - 1] = 0;
            index[CurrentDim - 1] < size[CurrentDim - 1];
            ++index[CurrentDim - 1])
        {
            GridLooper<Dim, CurrentDim - 1>::loop(size, index, f);
        }
    }
};

template <size_t Dim>
struct GridLooper<Dim, 0> {
    template <typename Func>
    static void loop(const Vec<Dim, int>&,
        Vec<Dim, int>&,
        Func&& f)
    {
        f();
    }
};

template <size_t Dim, typename T>
inline Mesh<Dim> GridToMesh<Dim, T>::convert(const Grid<Dim, T>& grid)
{
    Mesh<Dim> mesh;
    Vec<Dim, int> size = grid.getSize();
    int nbPoints = grid.getNbPoints();
    mesh.vertices.resize(nbPoints);
    Vec<Dim, int> index;

    GridLooper<Dim, Dim>::loop(size, index, [&]() {
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

    for (size_t d = 0; d < Dim; ++d)
    {
        Vec<Dim, int> neighbor = index;
        neighbor[d] += 1;

        if (neighbor[d] < size[d])
        {
            mesh.indices.push_back(currentIdx);
            mesh.indices.push_back(grid.flatten(neighbor));
        }
    }

}