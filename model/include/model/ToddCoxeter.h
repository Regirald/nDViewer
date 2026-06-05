#pragma once

#include "CoxeterPresentation.h"

#include <vector>
#include <queue>
#include <cstdint>

namespace Coxeter
{

    struct CosetTable
    {
        int generators = 0;

        std::vector<std::vector<int>> table;

        int newCoset();
    };

    struct UnionFind
    {
        std::vector<int> parent;

        int add();

        int find(int x);

        void unite(
            int a,
            int b);
    };

    struct Deduction
    {
        int source = -1;
        uint8_t generator = 0;
        int target = -1;
    };

    CosetTable enumerateCosets(
        const Presentation& presentation,
        const std::vector<Relator>& relators,
        const std::vector<uint8_t>& subgroupGenerators);

    void coincidence(
        CosetTable& table,
        UnionFind& uf,
        int a,
        int b);

}