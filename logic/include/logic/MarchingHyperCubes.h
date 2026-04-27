#pragma once
#include <cmath>
#include "core/Field.h"
#include "model/Mesh.h"
#include "model/ImplicitFunction.h"
#include "GridLooper.h"

template <size_t Dim>
class MarchingHyperCubes
{
	public:
		using Index = Vec<Dim, int>;
		static Vec<Dim, float> interp(const Vec<Dim, float>& a, const Vec<Dim, float>& b, float va, float vb, float iso)
		{
			float t = (iso - va) / (vb - va);
			return a + t * (b - a);
		}

		static Mesh<Dim> extractIsosurface(const ImplicitFunction<Dim>& impFunction,
			float iso,
			Grid<Dim, float> grid) 
		{
			Mesh<Dim> mesh;

			// --- 1. Precompute scalar field ---
			ScalarField<Dim> field(grid.m_resolution, grid.m_basis);
			GridLooper<Dim, Dim>::loop(grid.getSize(), [&](const Vec<Dim, int>& index) {
				field[index] = impFunction.evaluate(grid.getPosition(index));
				});

			// --- 2. Edge cache ---
			auto getEdgeVertex = [&](Index& idxA,Index& idxB)
				{
					Index idxV;

					Vec<Dim, float> p0 = grid.getPosition(idxA);
					Vec<Dim, float> p1 = grid.getPosition(idxB);

					float& v0 = field[idxA];
					float& v1 = field[idxB];

					Vec<Dim, float> p = interp(p0, p1, v0, v1, iso);

					int vid = (int)mesh.vertices.size();
					mesh.vertices.push_back(p);

					return vid;
				};

			// --- 3. Process hypercube ---
			auto processHyperCube = [&](Index& idxCube)
				{

					//consider a hypercube as a 2^Dim set of vertices, loop over them to determine inside/outside

				};
			return mesh;
		}

	
};