#pragma once
#include "CoxeterPresentation.h"
#include "ToddCoxeter.h"
#include "GeometryFromTopology.h"

// include your ToddCoxeter.cpp declarations
namespace Coxeter
{
    struct CosetTable;
    CosetTable enumerateCosets(
        const Presentation& presentation,
        const std::vector<Relator>& relators,
        const Subgroup& subgroupGenerators);
}

#include <iostream>
#include <cassert>

using namespace Coxeter;

// ----------------------------
// Helpers
// ----------------------------

Presentation makeCoxeter(const std::vector<int>& schlafli)
{
    return buildPresentation(schlafli);
}

std::vector<Relator> makeRelators(const Presentation& P)
{
    return buildRelators(P);
}


Subgroup makeVertexStabilizer(int rank)
{
    return buildVertexStabilizer(rank);
}
// ----------------------------
// Tests
// ----------------------------

void test_A1()
{
    Presentation P = makeCoxeter({});

    auto R = makeRelators(P);

    CosetTable T =
        enumerateCosets(P, R, {});

    std::cout << "[A1] cosets = " << T.table.size() << "\n";

    assert(T.table.size() == 2);
}

void test_A2()
{
    Presentation P = makeCoxeter({ 3 });
    auto R = makeRelators(P);

    CosetTable T =
        enumerateCosets(P, R, {});

    std::cout << "[A2] cosets = " << T.table.size() << "\n";

    assert(T.table.size() == 6);
}

void test_B2()
{
    Presentation P = makeCoxeter({ 4 });
    auto R = makeRelators(P);

    CosetTable T =
        enumerateCosets(P, R, {});

    std::cout << "[B2] cosets = " << T.table.size() << "\n";

    assert(T.table.size() == 8);
}

void test_A3()
{
    Presentation P = makeCoxeter({ 3,3 });
    auto R = makeRelators(P);

    CosetTable T =
        enumerateCosets(P, R, {});

    std::cout << "[A3] cosets = " << T.table.size() << "\n";

    assert(T.table.size() == 24);
}

void test_B3()
{
    const std::vector<int>& schlafli = { 4,3 };
    Presentation P = makeCoxeter(schlafli);
    auto R = makeRelators(P);
    Subgroup Hv = makeVertexStabilizer(P.rank);

    CosetTable T =
        enumerateCosets(P, R, Hv);

    CosetBFSData bfsData = buildCosetBFSData(T);


     auto initializersAndMirrors = buildInitializersAndMirrors<3>(P);


     Initializers<3> initializers = initializersAndMirrors.first;
     std::vector<Vec<3, float>> mirrors = initializersAndMirrors.second;

     Mesh<3> geom = generateGeometry<3>(
         bfsData,
         bfsData,
         initializers,
         mirrors);
    std::cout << "[B3] cosets = " << T.table.size() << "\n";

    assert(T.table.size() == 8);
}

void test_H3()
{
    Presentation P = makeCoxeter({ 5,3 });
    auto R = makeRelators(P);

    CosetTable T =
        enumerateCosets(P, R, {});

    std::cout << "[H3] cosets = " << T.table.size() << "\n";

    assert(T.table.size() == 120);
}

// ----------------------------
// Main
// ----------------------------

void test()
{
    std::cout << "Running Todd–Coxeter Coxeter tests...\n\n";

    //test_A1();
    //test_A2();
    //test_B2();
    //test_A3();
    test_B3();
    //test_H3();

    std::cout << "\nAll tests passed.\n";
}