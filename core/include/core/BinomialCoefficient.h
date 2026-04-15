#pragma once
#include <iostream>
#include <vector>


long long binomial(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k > n - k) k = n - k; // symmetry

    long long res = 1;
    for (int i = 1; i <= k; ++i) {
        res = res * (n - k + i) / i;
    }
    return res;
}

std::vector<std::vector<int>> generateCombinations(int n, int k) {
    std::vector<std::vector<int>> result;

    std::vector<int> comb(k);

    // initial combination: [0,1,2,...,k-1]
    for (int i = 0; i < k; ++i)
        comb[i] = i;

    while (true) {
        result.push_back(comb);

        // find position to increment
        int i = k - 1;
        while (i >= 0 && comb[i] == n - k + i)
            --i;

        if (i < 0)
            break;

        comb[i]++;

        for (int j = i + 1; j < k; ++j)
            comb[j] = comb[j - 1] + 1;
    }

    return result;
}
