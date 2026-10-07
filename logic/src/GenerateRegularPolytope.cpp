#include "GenerateRegularPolytope.h"


using namespace Coxeter;


PolytopeGeometry3D RegularPolytopeGenerator::generateRegularPolytope(std::vector<int>& schlafli)
{
    const std::vector<int>& schlafli_simplified = RegularPolytopeGenerator::simplifySchlafli(schlafli);
    switch (schlafli_simplified.size() + 1)
    {
    case 2:
        return RegularPolytopeGenerator::generateRegularPolytopeFromSimplifiedSchlafli<2>(schlafli_simplified);

    case 3:
        return RegularPolytopeGenerator::generateRegularPolytopeFromSimplifiedSchlafli<3>(schlafli_simplified);

    case 4:
        return RegularPolytopeGenerator::generateRegularPolytopeFromSimplifiedSchlafli<4>(schlafli_simplified);

    default:
        throw std::runtime_error("Unsupported dimension");
    }
}

template<size_t Dim>
PolytopeGeometry3D RegularPolytopeGenerator::generateRegularPolytopeFromSimplifiedSchlafli(const std::vector<int>& schlafli)
{
    Presentation Presentation = buildPresentation(schlafli);
    auto relators = buildRelators(Presentation);
    Subgroup Hv = buildVertexStabilizer(Presentation.rank);
    Subgroup He = buildEdgeStabilizer(Presentation.rank);
    std::vector<Subgroup> allStabilizers = buildAllStabilizers(Presentation.rank);
    
    std::vector<CosetTable> Tables;
    std::vector<CosetBFSData> costetsBFSData;
    for (auto stabilizer : allStabilizers)
    {
        CosetTable T = enumerateCosets(Presentation, relators, stabilizer);
        CosetBFSData bfsData = buildCosetBFSData(T);

        Tables.push_back(T);
        costetsBFSData.push_back(bfsData);
    }


	auto edgesAndFacesIndex = edgesAndFacesIndexFromTables(Tables[0], costetsBFSData, schlafli);

    auto initializersAndMirrors = buildInitializersAndMirrors<Dim>(Presentation);

    Initializers<Dim> initializers = initializersAndMirrors.first;
    std::vector<Vec<Dim, float>> mirrors = initializersAndMirrors.second;

    return generateGeometry<Dim>(
        costetsBFSData[0],
        initializers,
        mirrors, 
        edgesAndFacesIndex);

}
const std::vector<int> RegularPolytopeGenerator::simplifySchlafli(const std::vector<int>&schlafli)
{
    // Remove first zeroes (if any)
    std::vector<int> simplified = schlafli;
    while (!simplified.empty() && simplified.front() == 0)
    {
        simplified.erase(simplified.begin());
    }
    return simplified;
}

