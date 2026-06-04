#pragma once
#include <vector>
#include <queue>
#include <unordered_map>
#include <cstdint>

struct CosetTable
{
    int generators = 0;

    // table[c][g] = coset index
    std::vector<std::vector<int>> table;

    int newCoset()
    {
        table.emplace_back(generators, -1);
        return (int)table.size() - 1;
    }
};

struct Presentation
{
    int rank = 0;

    std::vector<std::vector<int>> M; // Coxeter matrix
};

struct Relator
{
    std::vector<uint8_t> word;
};

struct Deduction
{
    int coset;
    int generator;
};

struct UnionFind
{
    std::vector<int> parent;

    int add()
    {
        int id = parent.size();
        parent.push_back(id);
        return id;
    }

    int find(int x)
    {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int a, int b)
    {
        a = find(a);
        b = find(b);
        if (a != b) parent[b] = a;
    }
};

inline void link(
    CosetTable& T,
    int a,
    int g,
    int b)
{
    T.table[a][g] = b;
    T.table[b][g] = a;
}

void coincidence(
    CosetTable& T,
    UnionFind& UF,
    int a,
    int b)
{
    std::queue<std::pair<int, int>> Q;
    Q.push({ a,b });

    while (!Q.empty())
    {
        auto [x, y] = Q.front();
        Q.pop();

        x = UF.find(x);
        y = UF.find(y);

        if (x == y) continue;

        UF.unite(x, y);

        int rep = UF.find(x);

        for (int g = 0; g < T.generators; g++)
        {
            int tx = T.table[x][g];
            int ty = T.table[y][g];

            if (tx == -1 && ty == -1)
                continue;

            if (tx == -1)
            {
                T.table[rep][g] = ty;
                continue;
            }

            if (ty == -1)
            {
                T.table[rep][g] = tx;
                continue;
            }

            Q.push({ tx,ty });
        }
    }
}

void processDeduction(
    CosetTable& T,
    UnionFind& UF,
    std::queue<Deduction>& D,
    int c,
    int g)
{
    c = UF.find(c);

    if (T.table[c][g] != -1)
        return;

    // try to deduce from existing partial structure
    for (int h = 0; h < T.generators; h++)
    {
        int x = T.table[c][h];
        if (x == -1) continue;

        int y = T.table[x][g];

        if (y != -1)
        {
            coincidence(T, UF, c, x);
            return;
        }
    }

    // otherwise create new coset
    int newc = T.newCoset();
    UF.add();

    link(T, c, g, newc);

    D.push({ c,g });
    D.push({ newc,g });
}

void scanRelator(
    CosetTable& T,
    UnionFind& UF,
    std::queue<Deduction>& D,
    const Relator& R,
    int start)
{
    int current = start;

    for (auto g : R.word)
    {
        current = UF.find(current);

        if (T.table[current][g] == -1)
        {
            processDeduction(T, UF, D, current, g);
        }

        current = T.table[current][g];
    }

    coincidence(T, UF, current, start);
}

void enumerate(
    CosetTable& T,
    UnionFind& UF,
    const std::vector<Relator>& relators)
{
    std::queue<Deduction> D;

    bool changed = true;

    while (changed)
    {
        changed = false;

        size_t before = T.table.size();

        for (int c = 0; c < (int)T.table.size(); c++)
        {
            for (const auto& r : relators)
            {
                scanRelator(T, UF, D, r, c);
            }
        }

        // process deduction queue fully
        while (!D.empty())
        {
            auto d = D.front();
            D.pop();

            processDeduction(
                T, UF, D,
                d.coset,
                d.generator);
        }

        if (T.table.size() != before)
            changed = true;
    }
}

