#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#include "GramMatrixNormals.h"



Matrix cholesky(const Matrix& G)
{
    int n = G.size();
    Matrix L(n, Vector(n, 0.0));

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j <= i; ++j)
        {
            float sum = G[i][j];

            for (int k = 0; k < j; ++k)
                sum -= L[i][k] * L[j][k];

            if (i == j)
            {
                if (sum <= 0)
                    throw std::runtime_error("Gram matrix not positive definite.");

                L[i][j] = std::sqrt(sum);
            }
            else
            {
                L[i][j] = sum / L[j][j];
            }
        }
    }

    return L;
}

Matrix buildGramMatrix(const Coxeter::Presentation& P)
{
    int n = P.rank;
    Matrix G(n, std::vector<float>(n));

    for (int i = 0; i < n; ++i)
    {
        G[i][i] = 1.0;

        for (int j = i + 1; j < n; ++j)
        {
            float g = -std::cos(M_PI / P.M[i][j]);
            G[i][j] = G[j][i] = g;
        }
    }
    return G;
}

Vector buildV0(const Matrix& N)
{
    int n = N.size();
    Vector v0(n, 0.0);

    // Solve N * v0 = (1, 0, ..., 0)
    for (int i = 0; i < n; ++i)
    {
        float rhs = (i == 0) ? 1.0 : 0.0;

        for (int j = 0; j < i; ++j)
            rhs -= N[i][j] * v0[j];

        v0[i] = rhs / N[i][i];
    }

    return v0;
}



