#include <iostream>
#include <vector>
#include <cassert>

#include "ToddCoxeter.h"

Presentation makeCoxeter(const std::vector<int>& schlafli)
{
    Presentation P;
    P.rank = schlafli.size() + 1;

    P.M.assign(P.rank, std::vector<int>(P.rank, 2));

    for (size_t i = 0; i < P.rank; i++)
        P.M[i][i] = 1;

    for (size_t i = 0; i < schlafli.size(); i++)
    {
        P.M[i][i + 1] = schlafli[i];
        P.M[i + 1][i] = schlafli[i];
    }

    return P;
}

std::vector<Relator> buildRelators(const Presentation& P)
{
    std::vector<Relator> R;

    // involutions
    for (size_t i = 0; i < P.rank; i++)
        R.push_back(Relator{ {(uint8_t)i, (uint8_t)i} });

    // braid relations
    for (size_t i = 0; i < P.rank; i++)
    {
        for (size_t j = i + 1; j < P.rank; j++)
        {
            int m = P.M[i][j];
            if (m <= 2) continue;

            Relator r;

            for (int k = 0; k < m; k++)
            {
                r.word.push_back(i);
                r.word.push_back(j);
            }

            R.push_back(r);
        }
    }

    return R;
}

std::vector<int> extractVertices(const CosetTable& T)
{
    std::vector<int> v;
    for (int i = 0; i < (int)T.table.size(); i++)
        v.push_back(i);
    return v;
}

void validateEdges(
    const std::vector<std::pair<int, int>>& edges,
    int vertexCount)
{
    for (auto [a, b] : edges)
    {
        assert(a >= 0 && a < vertexCount);
        assert(b >= 0 && b < vertexCount);
        assert(a != b);
    }
}

void test_A2()
{
    auto P = makeCoxeter({ 3 });
    auto rel = buildRelators(P);

    UnionFind UF;
    CosetTable T = enumerate(P, rel, UF);

    std::cout << "[A2] cosets = "
        << T.table.size()
        << " (expected 6)\n";

    assert(T.table.size() == 6);
}