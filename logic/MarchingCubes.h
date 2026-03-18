#include <Mesh.h>
#include <Field.h>

class MarchingCubes {
public:
    static Mesh<3> extract(const ScalarField<3>& field, float iso) { return Mesh<3>(); };
};