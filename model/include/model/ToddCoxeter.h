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
        const Subgroup& subgroupGenerators);

    void defineFirstUndefinedEntry(
        CosetTable& table,
        UnionFind& uf,
        std::queue<Deduction>& deductions);

    int firstUndefinedCoset(
        const CosetTable& table,
        int& generator);

    void coincidence(
        CosetTable& table,
        UnionFind& uf,
        int a,
        int b);


    struct ScanResult
    {
        int forwardPos;
        int backwardPos;

        int forwardCoset;
        int backwardCoset;
    };

    ScanResult scanWord(
        CosetTable& table,
        UnionFind& uf,
        const Relator& relator,
        int startCoset);

    bool scanRelator(
        CosetTable& table,
        UnionFind& uf,
        const Relator& relator,
        int startCoset,
        std::queue<Deduction>& deductions);


    struct CosetBFSData
    {
        // BFS order of cosets, starting from 0
        std::vector<int> order;

        // parent coset in the BFS tree, -1 for root
        std::vector<int> parentCoset;

        // generator used to reach this coset from parent, -1 for root
        std::vector<int> parentGenerator;

        // shortest generator word reaching each coset from 0
        std::vector<std::vector<uint8_t>> representativeWord;
    };

    CosetBFSData buildCosetBFSData(const CosetTable& table);
}