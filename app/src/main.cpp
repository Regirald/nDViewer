// Core model / logic includes
#include "model/Circle.h"
#include "model/Mesh.h"
#include "logic/MarchingCubes.h"
#include "logic/MarchingSquares.h"
#include "view/Viewer.h"
#include <Windows.h>
#include <gl/GL.h>

int main()
{
    // 1. Create implicit model (placeholder for your computer vision source)
    Circle circle;

    // 2. Sample into scalar field
    int dimx = 64;
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
    Viewer viewer("Computer Vision Viewer", 720, 720);

    // Example: render callback that renders the extracted mesh using
    // legacy fixed-function OpenGL. This keeps things simple while
    // wiring data flow from marching cubes to the GPU.
    viewer.setRenderCallback([&](int w, int h)
        {
            if (h == 0) h = 1;

            glViewport(0, 0, w, h);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Projection
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-2, 2, -2, 2, -2, 2);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            // Draw mesh
            glBegin(GL_LINES);
            for (size_t i = 0; i < mesh.indices.size(); i += 3)
            {
                const auto& v0 = mesh.vertices[mesh.indices[i]];
                const auto& v1 = mesh.vertices[mesh.indices[i + 1]];
                const auto& v2 = mesh.vertices[mesh.indices[i + 2]];

                glVertex3f(v0[0], v0[1], 0.f);
                glVertex3f(v1[0], v1[1], 0.f);
                glVertex3f(v0[0], v0[1], 0.f);
                glVertex3f(v2[0], v2[1], 0.f);
                glVertex3f(v1[0], v1[1], 0.f);
                glVertex3f(v2[0], v2[1], 0.f);
            }
            glEnd();
        });

    viewer.run();
    return 0;
}
