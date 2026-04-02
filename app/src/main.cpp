// Core model / logic includes
#include "model/Circle.h"
#include "model/Mesh.h"
#include "logic/MarchingCubes.h"
#include "logic/MarchingSquares.h"
#include "view/Viewer.h"
#include <Windows.h>
#include <gl/GL.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 1. Create implicit model (placeholder for your computer vision source)
    Circle circle;

    // 2. Sample into scalar field
    int dimx = 20;
    int dimy = dimx;
    ScalarField<2> field({ dimx, dimy});
    // Fill field with 1.f inside the sphere and 0.f outside
    for (int y = 0; y < dimy; ++y)
    {
        for (int x = 0; x < dimx; ++x)
        {
            Vec<2, float> p;
            // Map indices from [0,63] to [-1,1]
            p[0] = (static_cast<float>(x) / (dimx - 1)) * 2.0f - 1.0f;
            p[1] = (static_cast<float>(y) / (dimy - 1)) * 2.0f - 1.0f;

            float value = circle.evaluate(p);

            Vec<2, int> idx;
            idx[0] = x;
            idx[1] = y;

            field(idx) = (value <= 0.0f) ? 1.0f : 0.0f;
        }
    }

    // 3. Extract topology
    Mesh<2> mesh = MarchingSquares::extract(field, 0.f);

    // 4. Open a window dedicated to computer vision visualization
    Viewer viewer;

    // Example: render callback that renders the extracted mesh using
    // legacy fixed-function OpenGL. This keeps things simple while
    // wiring data flow from marching cubes to the GPU.
    
	viewer.setMesh(&mesh);

    viewer.show();
    return app.exec(); // Qt event loop
}
