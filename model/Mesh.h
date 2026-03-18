#include <vector>
#include <Vec.h>

template <size_t Dim>
struct Mesh {
    std::vector<Vec<Dim, float>> vertices;
    std::vector<Vec<Dim, int>> indices;
};