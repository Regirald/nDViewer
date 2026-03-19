#include "Mesh.h"
#include "Field.h"

class MarchingSquares {
public:
    static Mesh<2> extract(const ScalarField<2>& field, float iso) { return Mesh<2>(); };
};