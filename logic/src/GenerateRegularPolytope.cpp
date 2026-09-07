#include "GenerateRegularPolytope.h"


using namespace Coxeter;


template<size_t Dim>
PolytopeGeometry<Dim> RegularPolytopeGenerator<Dim>::generateRegularPolytope(const std::vector<int>& schlafli)
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


template class RegularPolytopeGenerator<2>;
template class RegularPolytopeGenerator<3>;
template class RegularPolytopeGenerator<4>;
