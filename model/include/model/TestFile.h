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
    Presentation P = makeCoxeter({ 4,3 });
    auto R = makeRelators(P);
    Subgroup Hv = makeVertexStabilizer(P.rank);

    CosetTable T =
        enumerateCosets(P, R, Hv);

    CosetBFSData bfsData = buildCosetBFSData(T);



    std::vector<Vec<3, float>> mirrors(3);
    mirrors[0] = { 1,0,0 };

    mirrors[1] =
    {
        static_cast<float>(std::cos(3 * M_PI / 4)),
        static_cast<float>(std::sin(3 * M_PI / 4)),
         0
    };

    mirrors[2] =
    {
        0,
        static_cast<float>(std::cos(2 * M_PI / 3)),
        static_cast<float>(std::sqrt(1 - std::pow(std::cos(2 * M_PI / 3), 2)))
    };
    Vec<3, float> v0;
	//solve v0 such that mirror 0 is at distance 1, mirror 1 is at distance 0, mirror 2 is at distance 0
	v0[0] = 1;
	v0[1] = 0;
	v0[2] = 0;

    PolytopeGeometry<3> geom = generateGeometry<3>(
            bfsData,
            v0,
            mirrors);
    std::cout << "[B3] cosets = " << T.table.size() << "\n";

    assert(T.table.size() == 48);
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