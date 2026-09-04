#pragma once
#define _USE_MATH_DEFINES
#include <math.h>


#include "CoxeterPresentation.h"
#include "ToddCoxeter.h"
#include "Mesh.h"


template<size_t Dim>
using Matrix = Vec<Dim, Vec<Dim, float>>;
template<size_t Dim>
using Vector = Vec<Dim, float>;

template<size_t Dim>
struct PolytopeGeometry
{
    std::vector<Vector<Dim>> vertices;
    std::vector<std::vector<int>> edges;
    std::vector<std::vector<int>> faces;
    std::vector<std::vector<int>> cells;

    // ranks >= 4
    std::vector<std::vector<std::vector<int>>> higherRanks;
};




template<size_t Dim>
inline Matrix<Dim> cholesky(const Matrix<Dim>& G)
{
    Matrix<Dim> L(Vector<Dim>(0.0));

    for (int i = 0; i < Dim; ++i)
    {
        for (int j = 0; j <= i; ++j)
        {
            float sum = G[i][j];

            for (int k = 0; k < j; ++k)
                sum -= L[i][k] * L[j][k];

            if (i == j)
            {
                if (sum <= 0)
                    throw std::runtime_error("Gram matrix not positive definite.");

                L[i][j] = std::sqrt(sum);
            }
            else
            {
                L[i][j] = sum / L[j][j];
            }
        }
    }

    return L;
}

template<size_t Dim>
inline Matrix<Dim> buildGramMatrix(const Coxeter::Presentation& P)
{
    int n = P.rank;
    Matrix<Dim> G(Vector<Dim>(0.0));

    for (int i = 0; i < n; ++i)
    {
        G[i][i] = 1.0;

        for (int j = i + 1; j < n; ++j)
        {
            float g = -std::cos(M_PI / P.M[i][j]);
            G[i][j] = G[j][i] = g;
        }
    }
    return G;
}

template<size_t Dim>
inline Vector<Dim> buildV0(const Matrix<Dim>& N)
{
    Vector<Dim> v0(0.0);

    // Solve N * v0 = (1, 0, ..., 0)
    for (int i = 0; i < Dim; ++i)
    {
        float rhs = (i == 0) ? 1.0 : 0.0;

        for (int j = 0; j < i; ++j)
            rhs -= N[i][j] * v0[j];

        v0[i] = rhs / N[i][i];
    }

    return v0;
}



template<size_t Dim>
struct Initializers
{
    Vector<Dim> v0;
    std::pair<Vector<Dim>, Vector<Dim>> e;
    std::vector<Vector<Dim>> f;
};

template<size_t Dim>
inline Initializers<Dim> buildInitializers(Matrix<Dim> L, Vec<Dim, float> mirror0)
{
    Vector<Dim> v0 = buildV0<Dim>(L);
    Vector<Dim> v1 = reflect<Dim>(v0, mirror0);
	std::pair<Vector<Dim>, Vector<Dim>> e = std::make_pair(v0, v1);
	Initializers<Dim> initializers;
    initializers.v0 = v0;
	initializers.e = e;

    return initializers;
}

template<size_t Dim>
inline std::pair<Initializers<Dim>, std::vector<Vec<Dim, float>>> buildInitializersAndMirrors(const Coxeter::Presentation& presentation)
{
    Matrix<Dim> G = buildGramMatrix<Dim>(presentation);
    Matrix<Dim> L = cholesky(G);

    std::vector<Vec<Dim, float>> mirrors;
    for (int i = 0; i < presentation.rank; ++i)
    {
        Vec<Dim, float> normal;
        for (int j = 0; j < presentation.rank; ++j)
        {
            normal[j] = static_cast<float>(L[i][j]);
        }
        mirrors.push_back(normal);
    }

    Initializers<Dim> initializers = buildInitializers<Dim>(L, mirrors[0]);

    return std::pair<Initializers<Dim>, std::vector<Vec<Dim, float>>>(initializers, mirrors);
}



template<size_t Dim>
Mesh<Dim> fromPolytopeGeometryToMesh(const PolytopeGeometry<Dim>& poly)
{
	Mesh<Dim> mesh;
	mesh.vertices = poly.vertices;
    mesh.indices = 

	return mesh;
}

template<size_t Dim>
Vec<Dim, float> reflect(
    const Vec<Dim, float>& x,
    const Vec<Dim, float>& normal)
{
    double dot = 0.0;

    for (size_t i = 0;i < Dim;++i)
        dot += x[i] * normal[i];

    Vec<Dim, float> result;

    for (size_t i = 0;i < Dim;++i)
        result[i] =
        x[i] - 2.0 * dot * normal[i];

    return result;
}

template<size_t Dim>
std::vector<Vec<Dim, float>> applyWord(
    const std::vector<Vec<Dim, float>>& base,
    const std::vector<uint8_t>& word,
    const std::vector<Vec<Dim, float>>& mirrorNormals)
{
    std::vector<Vec<Dim, float>> baseCopy = base;

    for (uint8_t g : word)
    {
        for (Vec<Dim, float>& p : baseCopy)
        {
			p =
				reflect(
					p,
					mirrorNormals[g]);
        }
    }

    return baseCopy;
}

template<size_t Dim>
PolytopeGeometry<Dim> generateGeometry(
    const Coxeter::CosetBFSData& bfsDataVertex,
    const Coxeter::CosetBFSData& bfsDataEdge,
    const Initializers<Dim>& initializers,
    const std::vector<Vec<Dim, float>>& mirrorNormals,
    const std::vector<std::vector<int>>& edgesIndex)
{
    PolytopeGeometry<Dim> poly;

    const size_t nbVertex = bfsDataVertex.representativeWord.size();
    const size_t nbEdge = bfsDataEdge.representativeWord.size();

    poly.vertices.resize(nbVertex);

    for (size_t i = 0;i < nbVertex;++i)
    {
        poly.vertices[i] =
            applyWord(
                {initializers.v0},
                bfsDataVertex.representativeWord[i],
                mirrorNormals)[0];
    }

    poly.edges = edgesIndex;

    return poly;
}

