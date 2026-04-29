#include <Windows.h>
#include <gl/GL.h>
#include <QApplication>
// Core model / logic includes
#include "model/Sphere.h"
#include "model/Mesh.h"
#include "core/SimplexGrid.h"
#include "core/Resolution.h"
#include "core/OrthonormalBasis.h"

#include "logic/GridToMesh.h"
#include "logic/MarchingCubes.h"
#include "logic/MarchingHyperCubes.h"
#include "logic/MarchingSquares.h"

#include "view/Viewer.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 1. Create implicit model (placeholder for your computer vision source)
    Sphere sphere;

    // 2. Sample into scalar field
    int res = 10;
	Resolution<2> resolution({ res, res});
	Resolution<3> resolution3({ res, res, res});

    // 3. Extract topology
	SimplexGrid<2> orthoGrid(resolution, OrthonormalBasis<2>());
	SimplexGrid<3> orthoGrid3(resolution3, OrthonormalBasis<3>());
    Mesh<3> mesh = MarchingHyperCubes<3>::extractIsosurface(sphere, 0.f, orthoGrid3);
	//Mesh<2> gridMesh2 = GridToMesh<2, float>::convert(orthoGrid);
	Mesh<3> gridMesh3 = GridToMesh<3, float>::convert(orthoGrid3);

    // 4. Open a window dedicated to computer vision visualization
    Viewer viewer;    
	viewer.setMesh(&mesh);
	viewer.setMesh(&gridMesh3);

    viewer.show();
    return app.exec(); // Qt event loop
}
