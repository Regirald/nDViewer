#include "GenerateRegularPolytope.h"


using namespace Coxeter;


template<size_t Dim>
Mesh<Dim> RegularPolytopeGenerator<Dim>::generateRegularPolytope(const std::vector<int>& schlafli)
{
    Presentation Presentation = buildPresentation(schlafli);
    auto relators = buildRelators(Presentation);
    Subgroup Hv = buildVertexStabilizer(Presentation.rank);

    CosetTable T =
        enumerateCosets(Presentation, relators, Hv);

    CosetBFSData bfsData = buildCosetBFSData(T);

    auto v0_and_mirrors = buildV0andMirrors<Dim, float>(Presentation);

    Vec<Dim, float> v0 = v0_and_mirrors.first;
    std::vector<Vec<Dim, float>> mirrors = v0_and_mirrors.second;

    return generateGeometry<Dim>(
        bfsData,
        v0,
        mirrors);
}


template class RegularPolytopeGenerator<2>;
template class RegularPolytopeGenerator<3>;
template class RegularPolytopeGenerator<4>;
