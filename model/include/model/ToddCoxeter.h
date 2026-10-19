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

        std::vector<int> cosetEquivalenceList;
        std::queue<int> coincidenceQueue;

        void newCoset();
    };


    struct Deduction
    {
        int source = -1;
        uint8_t generator = 0;
        int target = -1;
    };


    CosetTable enumerateCosets(
        const Presentation& presentation,
        const std::vector<Generator>& relators,
        const Subgroup& subgroupGenerators);


    int firstUndefinedCoset(
        const CosetTable& table,
        int& generator);


    struct ScanResult
    {
        int forwardPos;
        int backwardPos;

        int forwardCoset;
        int backwardCoset;
    };

    ScanResult scanWord(
        CosetTable& table,
        const Generator& relator,
        int startCoset);

    void scanGenerator(
        CosetTable& table,
        const Generator& relator,
        int startCoset);



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


    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> edgesAndFacesIndexFromTables(const CosetTable& vertexTable, std::vector<CosetBFSData> bfsData, const std::vector<int>& schlafli);

}