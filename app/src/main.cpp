// Core model / logic includes
#include "model/Circle.h"
#include "model/Mesh.h"
#include "core/Resolution.h"
#include "core/SimplexGrid.h"
#include "logic/MarchingCubes.h"
#include "logic/GridToMesh.h"
#include "logic/MarchingSquares.h"
#include "view/Viewer.h"
#include <Windows.h>
#include <gl/GL.h>
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 1. Create implicit model (placeholder for your computer vision source)
    Circle circle;

    // 2. Sample into scalar field
	Resolution<2> resolution({ 10, 10 });
	Resolution<3> resolution3({ 10, 10 , 10});

    // 3. Extract topology
	SimplexGrid<2, float> simplexGrid(resolution, SimplexBasis<2>());
	SimplexGrid<3, float> simplexGrid3(resolution3, SimplexBasis<3>());
    Mesh<2> mesh = MarchingSquares::extractContour(circle, 0.f, simplexGrid);
	Mesh<3> gridMesh = GridToMesh<3, float>::convert(simplexGrid3);
    // 4. Open a window dedicated to computer vision visualization
    Viewer viewer;    
	viewer.setMesh(&mesh);
	viewer.setMesh(&gridMesh);

    viewer.show();
    return app.exec(); // Qt event loop
}
