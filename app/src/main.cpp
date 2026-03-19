// Core model / logic includes
#include "model/Sphere.h"
#include "model/Mesh.h"
#include "logic/MarchingCubes.h"
#include "view/Viewer.h"

int main() {
    // 1. Create implicit model (placeholder for your computer vision source)
    Sphere sphere;

    // 2. Sample into scalar field
    ScalarField<3> field({ 64, 64, 64 });
    // TODO: fill field from your computer vision pipeline

    // 3. Extract topology
    Mesh<3> mesh = MarchingCubes::extract(field, 0.0f);

    // 4. Open a window dedicated to computer vision visualization
    Viewer viewer("Computer Vision Viewer", 1280, 720);
    viewer.setMesh(mesh);
    viewer.run();

    return 0;
}
