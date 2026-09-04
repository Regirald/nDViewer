#pragma once

#include <vector>
#include <cstdint>

namespace Coxeter
{

    struct Presentation
    {
        int rank = 0;

        // Coxeter matrix
        // M[i][j] = order of s_i s_j
        std::vector<std::vector<int>> M;
    };

    struct Relator
    {
        std::vector<uint8_t> word;
    };
    struct Subgroup
    {
        std::vector<int> generators;
    };

    Presentation buildPresentation(
        const std::vector<int>& schlafli);

    std::vector<Relator> buildRelators(
        const Presentation& presentation);

    Subgroup buildVertexStabilizer(
		int rank);

    Subgroup buildEdgeStabilizer(
		int rank);
}


//std::vector<uint8_t>
//makeVertexStabilizer(
//    int rank);

std::vector<uint8_t>
makeEdgeStabilizer(
    int rank);

std::vector<uint8_t>
makeFaceStabilizer(
    int rank);

std::vector<uint8_t>
makeCellStabilizer(
    int rank);