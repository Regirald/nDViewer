#pragma once
#include "ToddCoxeter.h"

struct Subgroup
{
    std::vector<int> generators;
};

struct CosetSpace
{
    std::vector<int> representatives;
};

std::vector<int> computeCosets(
    const CosetTable& T,
    const Subgroup& H)
{
    std::vector<int> cosets;

    for (int i = 0; i < (int)T.table.size(); i++)
    {
        cosets.push_back(i);
    }

    return cosets;
}

