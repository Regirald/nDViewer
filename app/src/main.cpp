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
	Resolution<2> resolution({ 4, 4 });


    // 3. Extract topology
    Mesh<2> mesh = MarchingSquares::extractContour(circle, -0.5f, resolution);
	SimplexGrid<2, float> simplexGrid(resolution, SimplexBasis<2>());
	Mesh<2> gridMesh = GridToMesh<2, float>::convert(simplexGrid);
    // 4. Open a window dedicated to computer vision visualization
    Viewer viewer;    
	viewer.setMesh(&mesh);
	viewer.setMesh(&gridMesh);

    viewer.show();
    return app.exec(); // Qt event loop
}
