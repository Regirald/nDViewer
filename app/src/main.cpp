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
    ScalarField<2> field({ 64, 64});
    // Fill field with 1.f inside the sphere and 0.f outside
    for (int y = 0; y < 64; ++y)
    {
        for (int x = 0; x < 64; ++x)
        {
            Vec<2, float> p;
            // Map indices from [0,63] to [-1,1]
            p[0] = (static_cast<float>(x) / 63.0f) * 2.0f - 1.0f;
            p[1] = (static_cast<float>(y) / 63.0f) * 2.0f - 1.0f;

            float value = circle.evaluate(p);

            Vec<2, int> idx;
            idx[0] = x;
            idx[1] = y;

            field(idx) = (value <= 0.0f) ? 1.0f : 0.0f;
        }
    }

    // 3. Extract topology
    Mesh<2> mesh = MarchingSquares::extract(field, 0.5f);

    // 4. Open a window dedicated to computer vision visualization
    Viewer viewer("Computer Vision Viewer", 1280, 720);

    // Example: render callback that renders the extracted mesh using
    // legacy fixed-function OpenGL. This keeps things simple while
    // wiring data flow from marching cubes to the GPU.
    viewer.setRenderCallback([&](int width, int height)
    {
        if (height <= 0) height = 1;

        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Simple view transform: move back and look at origin.
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -3.0f);
        glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(30.0f, 0.0f, 1.0f, 0.0f);

        // Draw mesh as a triangle list.
        if (!mesh.empty())
        {
            glColor3f(0.2f, 0.7f, 1.0f);
            glBegin(GL_TRIANGLES);
            for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
            {
                unsigned int i0 = mesh.indices[i + 0];
                unsigned int i1 = mesh.indices[i + 1];
                unsigned int i2 = mesh.indices[i + 2];
                const Vec<2, float>& v0 = mesh.vertices[i0];
                const Vec<2, float>& v1 = mesh.vertices[i1];
                const Vec<2, float>& v2 = mesh.vertices[i2];
                glVertex3f(v0[0], v0[1], 0.f);
                glVertex3f(v1[0], v1[1], 0.f);
                glVertex3f(v2[0], v2[1], 0.f);
            }
            glEnd();
        }
    });

    viewer.run();
    return 0;
}
