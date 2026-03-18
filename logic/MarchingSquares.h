#include <vector>
#include <Vec.h>
#include <Field.h>

class MarchingSquares {
public:
    static std::vector<std::vector<Vec<2, float>>>
        extract(const ScalarField<2>& field, float iso);
};