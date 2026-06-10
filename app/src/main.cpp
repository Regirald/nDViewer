#define NOMINMAX
#define QT_NO_DEPRECATED_WARNINGS
#include <Windows.h>
#include <gl/GL.h>
#include <QApplication>
#include <memory>
// Core model / logic includes
#include "model/Sphere.h"
#include "model/Circle.h"
#include "model/Mesh.h"

#include "model/CoxeterPresentation.h"
#include "model/ToddCoxeter.h"
#include "model/TestFile.h"

#include "core/SimplexGrid.h"
#include "core/Resolution.h"
#include "core/OrthonormalBasis.h"

#include "logic/GridToMesh.h"
#include "logic/MarchingCubes.h"
#include "logic/MarchingHyperCubes.h"
#include "logic/MarchingSquares.h"
#include "logic/GenerateRegularPolytope.h"

#include "view/Viewer.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 1. Create implicit model (placeholder for your computer vision source)
    Sphere sphere0;
    Sphere sphere1({ 0.5f, 0.5f, 0.5f }, 1.f);
    Sphere sphere2({ 2.f, 2.f, 2.f }, 2.f);
    // Create shared_ptrs and cast to base type so operator+ overload is found
    auto s0ptr = std::make_shared<Sphere>(sphere0);
    auto s1ptr = std::make_shared<Sphere>(sphere1);
	auto s2ptr = std::make_shared<Sphere>(sphere2);
    auto sphere3 = std::static_pointer_cast<ImplicitFunction<3>>(s0ptr)
        - std::static_pointer_cast<ImplicitFunction<3>>(s1ptr);; // Union of two spheres
    Circle circle;

    // 2. Sample into scalar field
    int res = 5;
    Resolution<2> resolution2({ res, res});
    Resolution<3> resolution3({ res, res, res});

    // 3. Extract topology
    SimplexGrid<2> orthoGrid2(resolution2, OrthonormalBasis<2>());
    SimplexGrid<3> orthoGrid3(resolution3, OrthonormalBasis<3>());
    //Mesh<2> mesh2 = MarchingHyperCubes<2>::extractIsosurface(circle, 0.f, orthoGrid2);
    //Mesh<3> mesh3 = MarchingHyperCubes<3>::extractIsosurface(sphere0, 0.f, orthoGrid3);
    //Mesh<2> gridMesh2 = GridToMesh<2, float>::convert(orthoGrid);
    Mesh<2> gridMesh2 = GridToMesh<2, float>::convert(orthoGrid2);
    Mesh<3> gridMesh3 = GridToMesh<3, float>::convert(orthoGrid3);


    /*
    Schläfli symbol
        ↓
        Coxeter matrix
        ↓
        Coxeter group presentation
        ↓
        Word reduction engine
        ↓
        Group enumeration(Todd–Coxeter or equivalent)
        ↓
        Parabolic subgroup generation
        ↓
        Coset enumeration
        ↓
        Incidence extraction
        ↓
        Vertices / edges / faces / cells
     */


    auto P =
        Coxeter::buildPresentation(
            { 4,3 });

    auto relators =
        Coxeter::buildRelators(P);

    std::cout
        << "Rank = "
        << P.rank
        << std::endl;

    std::cout
        << "Relators = "
        << relators.size()
        << std::endl;

    test();


    //auto polygon = RegularPolytopeGenerator<3>::generateRegularPolytope({ 4, 3 });

    // 4. Open a window dedicated to computer vision visualization
    Viewer viewer;    
    //viewer.setMesh(&polygon);
    //viewer.setMesh(&mesh3);
    //viewer.setImplicitFunction(sphere3.get());
    //viewer.setImplicitFunction(&sphere1);
    // To use the union implicit function instead, pass sphere2.get()
    // viewer.setImplicitFunction(sphere2.get());
    //viewer.setMesh(&gridMesh2);
    //viewer.setMesh(&gridMesh3);

    viewer.show();
    return app.exec(); // Qt event loop
}
