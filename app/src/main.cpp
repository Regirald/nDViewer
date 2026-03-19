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
    // TODO: fill field from your vision pipeline.

    // 3. Extract topology
    Mesh<3> mesh = MarchingCubes::extract(field, 0.0f);

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
