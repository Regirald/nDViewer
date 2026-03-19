// Core model / logic includes
#include "model/Sphere.h"
#include "model/Mesh.h"
#include "logic/MarchingCubes.h"
#include "view/Viewer.h"
#include <Windows.h>
#include <gl/GL.h>

int main()
{
    // 1. Create implicit model (placeholder for your computer vision source)
    Sphere sphere;

    // 2. Sample into scalar field
    ScalarField<3> field({ 64, 64, 64 });
    // Fill field with 1.f inside the sphere and 0.f outside
    for (int z = 0; z < 64; ++z)
    {
        for (int y = 0; y < 64; ++y)
        {
            for (int x = 0; x < 64; ++x)
            {
                Vec<3, float> p;
                // Map indices from [0,63] to [-1,1]
                p[0] = (static_cast<float>(x) / 63.0f) * 2.0f - 1.0f;
                p[1] = (static_cast<float>(y) / 63.0f) * 2.0f - 1.0f;
                p[2] = (static_cast<float>(z) / 63.0f) * 2.0f - 1.0f;

                float value = sphere.evaluate(p);

                Vec<3, int> idx;
                idx[0] = x;
                idx[1] = y;
                idx[2] = z;

                field(idx) = (value <= 0.0f) ? 1.0f : 0.0f;
            }
        }
    }

    // 3. Extract topology
    Mesh<3> mesh = MarchingCubes::extract(field, 0.5f);

    // 4. Open a window dedicated to computer vision visualization
    Viewer viewer("Computer Vision Viewer", 1280, 720);

    // Example: render callback that just clears with a color depending on size.
    viewer.setRenderCallback([&](int width, int height)
    {
        // Here you can upload textures, render meshes, etc.
        // For now, simple clear color change as a stub.
        float r = (width  % 256) / 255.0f;
        float g = (height % 256) / 255.0f;
        glClearColor(r, g, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    });

    viewer.run();
    return 0;
}
