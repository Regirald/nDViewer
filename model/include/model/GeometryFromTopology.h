// CoxeterEmbedding.h
#pragma once
#include <vector>
#include <array>
#include <cmath>

#include "core/Vec.h"
#include "CoxeterPresentation.h"
#include "ToddCoxeter.h"



template<size_t Dim>
struct PolytopeGeometry
{
    std::vector<Vec<Dim, float>> vertices;
};

template<size_t Dim>
Mesh<Dim> fromPolytopeGeometryToMesh(const PolytopeGeometry<Dim>& poly)
{
	Mesh<Dim> mesh;
	mesh.vertices = poly.vertices;
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
Vec<Dim, float> applyWord(
    const Vec<Dim, float>& baseVertex,
    const std::vector<uint8_t>& word,
    const std::vector<Vec<Dim, float>>& mirrorNormals)
{
    Vec<Dim, float> p = baseVertex;

    for (uint8_t g : word)
    {
        p =
            reflect(
                p,
                mirrorNormals[g]);
    }

    return p;
}

template<size_t Dim>
Mesh<Dim>
generateGeometry(
    const Coxeter::CosetBFSData& bfsData,
    const Vec<Dim, float>& baseVertex,
    const std::vector<Vec<Dim, float>>& mirrorNormals)
{
    PolytopeGeometry<Dim> poly;

    const size_t count =
        bfsData.representativeWord.size();

    poly.vertices.resize(count);

    for (size_t i = 0;i < count;++i)
    {
        poly.vertices[i] =
            applyWord(
                baseVertex,
                bfsData.representativeWord[i],
                mirrorNormals);
    }

    return fromPolytopeGeometryToMesh(poly);
}