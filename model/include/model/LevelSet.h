#pragma once
template <size_t Dim>
class LevelSet {
public:
    ScalarField<Dim> field;

    float isoValue = 0.0f;
};