#include "GenerateRegularPolytope.h"


using namespace Coxeter;


template<size_t Dim>
PolytopeGeometry<Dim> RegularPolytopeGenerator<Dim>::generateRegularPolytope(const std::vector<int>& schlafli)
{
    Presentation Presentation = buildPresentation(schlafli);
    auto relators = buildRelators(Presentation);
    Subgroup Hv = buildVertexStabilizer(Presentation.rank);
    Subgroup He = buildEdgeStabilizer(Presentation.rank);

    CosetTable Tv = enumerateCosets(Presentation, relators, Hv);
    CosetTable Te = enumerateCosets(Presentation, relators, He);

    CosetBFSData bfsDataVertex = buildCosetBFSData(Tv);
    CosetBFSData bfsDataEdge = buildCosetBFSData(Te);

	std::vector<std::vector<int>> edgesIndex = edgesIndexFromTable(Tv, bfsDataVertex, Te, bfsDataEdge);

    auto initializersAndMirrors = buildInitializersAndMirrors<Dim>(Presentation);

    Initializers<Dim> initializers = initializersAndMirrors.first;
    std::vector<Vec<Dim, float>> mirrors = initializersAndMirrors.second;

    return generateGeometry<Dim>(
        bfsDataVertex,
        bfsDataEdge,
        initializers,
        mirrors, 
        edgesIndex);
}


template class RegularPolytopeGenerator<2>;
template class RegularPolytopeGenerator<3>;
template class RegularPolytopeGenerator<4>;
