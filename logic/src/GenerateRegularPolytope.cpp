#include <queue>
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>
#include <set>
#include <map>

#include "GenerateRegularPolytope.h"

constexpr double GRP_EPS = 1e-6;


template<size_t Dim>
typename RegularPolytopeGenerator<Dim>::Vec
RegularPolytopeGenerator<Dim>::reflect(const Vec& v, const Vec& n)
{
    // ensure float scalar multiplication
    float s = static_cast<float>(2.0 * dot(v, n));
    return v - s * n;
}

template<size_t Dim>
bool RegularPolytopeGenerator<Dim>::equalVec(const Vec& a, const Vec& b)
{
    for (size_t i = 0; i < Dim; ++i)
    {
        if (std::abs(a[i] - b[i]) > GRP_EPS)
            return false;
    }

    return true;
}

template<size_t Dim>
int RegularPolytopeGenerator<Dim>::findVertex(const std::vector<Vec>& verts, const Vec& v)
{
    for (int i = 0; i < (int)verts.size(); ++i)
    {
        if (equalVec(verts[i], v))
            return i;
    }

    return -1;
}



template<size_t Dim>
std::vector<std::vector<double>>
RegularPolytopeGenerator<Dim>::buildGram(const std::vector<std::vector<int>>& m)
{
    int n = (int)m.size();

    std::vector<std::vector<double>> G(
        n,
        std::vector<double>(n, 0.0));

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            if (i == j)
                G[i][j] = 1.0;
            else
                G[i][j] = -std::cos(M_PI / m[i][j]);
        }
    }

    return G;
}

template<size_t Dim>
std::vector<Vec<Dim, float>> RegularPolytopeGenerator<Dim>::buildSimpleRoots(const std::vector<std::vector<double>>& G)
{
    int n = (int)G.size();

    std::vector<Vec> roots(n, Vec());

    // Cholesky-like construction

    for (int i = 0; i < n; ++i)
    {
        roots[i][i] = 1.0;

        for (int j = 0; j < i; ++j)
        {
            roots[i][j] = G[i][j];

            for (int k = 0; k < j; ++k)
                roots[i][j] -= roots[i][k] * roots[j][k];

            roots[i][j] /= roots[j][j];
        }

        double s = 0.0;

        for (int k = 0; k < i; ++k)
            s += roots[i][k] * roots[i][k];

        roots[i][i] = std::sqrt(std::max(1.0 - s, 0.0));
    }

    for (auto& r : roots)
        r = normalize(r);

    return roots;
}

template<size_t Dim>
Mesh<Dim> RegularPolytopeGenerator<Dim>::generateRegularPolytope(const std::vector<int>& schlafli)
{
    int rank = Dim;

    CoxeterSystem<Dim> sys;
    sys.rank = rank;

    sys.m.resize(rank, std::vector<int>(rank, 2));

    for (int i = 0; i < rank; ++i)
        sys.m[i][i] = 1;

    for (int i = 0; i < (int)schlafli.size(); ++i)
    {
        sys.m[i][i + 1] = schlafli[i];
        sys.m[i + 1][i] = schlafli[i];
    }

    auto G = buildGram(sys.m);

    sys.roots = buildSimpleRoots(G);

    Mesh<Dim> mesh;

    // Seed point
    Vec seed;

    for (size_t i = 0; i < Dim; ++i)
        seed[i] = 1.0f;

    seed = normalize(seed);

    std::queue<int> q;

    mesh.vertices.push_back(seed);
    q.push(0);
    int iter = 0;
	//TODO: only used to visualise roots, remove later
    for (int r = 0; r < (int)sys.roots.size(); ++r)
    {
        int idx = (int)mesh.vertices.size();
        mesh.vertices.push_back(sys.roots[r]);
	}
    while (!q.empty() && iter < 100)
    {
        int vi = q.front();
        q.pop();

        Vec v = mesh.vertices[vi];

        for (int r = 0; r < rank && r < (int)sys.roots.size(); ++r)
        {
            Vec vr = reflect(v, sys.roots[r]);

            int idx = findVertex(mesh.vertices, vr);

            if (idx < 0)
            {
                iter++;
                idx = (int)mesh.vertices.size();
                mesh.vertices.push_back(vr);
                q.push(idx);
            }

            if (vi != idx)
            {
                int a = std::min(vi, idx);
                int b = std::max(vi, idx);

                // store edges as index pairs in mesh.indices (two entries per edge)
                mesh.indices.push_back(static_cast<unsigned int>(a));
                mesh.indices.push_back(static_cast<unsigned int>(b));
            }
        }
    }

    // remove duplicate edges (pairs)
    std::set<std::pair<int, int>> eset;
    for (size_t i = 0; i + 1 < mesh.indices.size(); i += 2)
    {
        int a = static_cast<int>(mesh.indices[i]);
        int b = static_cast<int>(mesh.indices[i + 1]);
        if (a > b) std::swap(a, b);
        eset.insert({ a,b });
    }

    mesh.indices.clear();
    for (const auto& e : eset)
    {
        mesh.indices.push_back(static_cast<unsigned int>(e.first));
        mesh.indices.push_back(static_cast<unsigned int>(e.second));
    }

    return mesh;
}


template class RegularPolytopeGenerator<2>;
template class RegularPolytopeGenerator<3>;
template class RegularPolytopeGenerator<4>;
