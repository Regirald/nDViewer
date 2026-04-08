#pragma once

#include "Vec.h"

template <size_t Dim>
class Resolution
{
public:
	Resolution(Vec<Dim, int> size_) { size = size_; }

	Vec<Dim, int> size;
};
