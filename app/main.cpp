#include <Sphere.h>
#include <MarchingCubes.h>

int main() {
    // 1. Create implicit model
    Sphere sphere;

    // 2. Sample into scalar field
    ScalarField<3> field({ 64, 64, 64 });
    // fill field...

    // 3. Extract topology
    Mesh<3> mesh = MarchingCubes::extract(field, 0.0f);

    // 4. Send to viewer
/*    Viewer viewer;
    viewer.setMesh(mesh);
    viewer.run();*/
}