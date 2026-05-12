#pragma once
#include <vector>
#include "model/Mesh.h"

template <size_t Dim>
struct CoxeterSystem
{
    int rank;
    std::vector<std::vector<int>> m;
    std::vector<Vec<Dim, float>> roots;
};

template <size_t Dim>
class RegularPolytopeGenerator
{
public:
    using Vec = Vec<Dim, float>;
    static Mesh<Dim> generateRegularPolytope(const std::vector<int>& schlafli);

    static std::vector<std::vector<double>> buildGram(const std::vector<std::vector<int>>& m);
    static std::vector<Vec> buildSimpleRoots(const std::vector<std::vector<double>>& G);

private:
    static Vec reflect(const Vec& v, const Vec& n);
    static bool equalVec(const Vec& a, const Vec& b);
    static int findVertex(const std::vector<Vec>& verts, const Vec& v);
};


