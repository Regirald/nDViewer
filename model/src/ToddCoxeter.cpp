// ToddCoxeter.cpp
#include "ToddCoxeter.h"
#include "CoxeterPresentation.h"

#include <vector>
#include <queue>
#include <cstdint>
#include <algorithm>
#include <stdexcept>

namespace Coxeter
{



    int UnionFind::add()
    {
        const int id = static_cast<int>(parent.size());
        parent.push_back(id);
        return id;
    }

    int UnionFind::find(int x)
    {
        if (x < 0 || x >= static_cast<int>(parent.size()))
            throw std::out_of_range("UnionFind::find: index out of range");

        if (parent[x] == x)
            return x;

        parent[x] = find(parent[x]);
        return parent[x];
    }

    void UnionFind::unite(int a, int b)
    {
        a = find(a);
        b = find(b);

        if (a != b)
            parent[b] = a;
    }




    namespace detail
    {

        static void rawLink(CosetTable& table, int a, uint8_t g, int b)
        {
            table.table[a][g] = b;
            table.table[b][g] = a;
        }

        static void coincidence(
            CosetTable& table,
            UnionFind& uf,
            int a,
            int b,
            std::queue<Deduction>& deductions,
            bool& changed)
        {
            std::queue<std::pair<int, int>> pending;
            pending.push({ a, b });

            while (!pending.empty())
            {
                auto [x0, y0] = pending.front();
                pending.pop();

                int x = uf.find(x0);
                int y = uf.find(y0);

                if (x == y)
                    continue;

                uf.unite(x, y);
                changed = true;

                const int rep = uf.find(x);
                const int other = (rep == x) ? y : x;

                for (int g = 0; g < table.generators; ++g)
                {
                    int tx = table.table[x][g];
                    int ty = table.table[y][g];

                    if (tx == -1 && ty == -1)
                        continue;

                    if (tx == -1)
                    {
                        table.table[rep][g] = uf.find(ty);
                        table.table[uf.find(ty)][g] = rep;
                        continue;
                    }

                    if (ty == -1)
                    {
                        table.table[rep][g] = uf.find(tx);
                        table.table[uf.find(tx)][g] = rep;
                        continue;
                    }

                    tx = uf.find(tx);
                    ty = uf.find(ty);

                    if (tx != ty)
                        pending.push({ tx, ty });
                }

                (void)other;
            }

            while (!deductions.empty())
            {
                Deduction d = deductions.front();
                deductions.pop();

                d.source = uf.find(d.source);
                d.target = uf.find(d.target);

                if (d.source == d.target)
                    continue;

                int existing = table.table[d.source][d.generator];

                if (existing == -1)
                {
                    rawLink(table, d.source, d.generator, d.target);
                    changed = true;
                    continue;
                }

                existing = uf.find(existing);

                if (existing != d.target)
                    pending.push({ existing, d.target });
            }
        }

        static void processDeduction(
            CosetTable& table,
            UnionFind& uf,
            const Deduction& d,
            std::queue<Deduction>& deductions,
            bool& changed)
        {
            int source = uf.find(d.source);
            int target = uf.find(d.target);
            const uint8_t g = d.generator;

            //if (source == target)
            //    return;

            if (table.table[source][g] == -1)
            {
                rawLink(table, source, g, target);
                changed = true;
                return;
            }

            int existing = uf.find(table.table[source][g]);

            if (existing == target)
                return;

            coincidence(table, uf, existing, target, deductions, changed);
        }

        static void processAllDeductions(
            CosetTable& table,
            UnionFind& uf,
            std::queue<Deduction>& deductions,
            bool& changed)
        {
            while (!deductions.empty())
            {
                Deduction d = deductions.front();
                deductions.pop();

                processDeduction(table, uf, d, deductions, changed);
            }
        }


        static void compressTable(CosetTable& table, UnionFind& uf)
        {
            std::vector<int> repToNew(table.table.size(), -1);
            int nextId = 0;

            for (int i = 0; i < static_cast<int>(table.table.size()); ++i)
            {
                const int rep = uf.find(i);
                if (repToNew[rep] == -1)
                    repToNew[rep] = nextId++;
            }

            std::vector<std::vector<int>> newTable(
                nextId,
                std::vector<int>(table.generators, -1));

            for (int i = 0; i < static_cast<int>(table.table.size()); ++i)
            {
                const int rep = uf.find(i);
                const int row = repToNew[rep];

                for (int g = 0; g < table.generators; ++g)
                {
                    int tgt = table.table[i][g];
                    if (tgt == -1)
                        continue;

                    tgt = uf.find(tgt);
                    newTable[row][g] = repToNew[tgt];
                }
            }

            table.table = std::move(newTable);
        }

        static bool hasUndefinedEntries(const CosetTable& table)
        {
            for (const auto& row : table.table)
            {
                for (int v : row)
                {
                    if (v == -1)
                        return true;
                }
            }
            return false;
        }

    } // namespace detail

    int CosetTable::newCoset()
    {
        table.emplace_back(generators, -1);
        return static_cast<int>(table.size()) - 1;
    }

    CosetTable enumerateCosets(
        const Presentation& presentation,
        const std::vector<Relator>& relators,
        const Subgroup& subgroup)
    {
        CosetTable table;
        table.generators = presentation.rank;

        UnionFind uf;
        std::queue<Deduction> deductions;

        table.newCoset();
        uf.add();

        for (int g : subgroup.generators)
        {
            deductions.push(
                {
                    0,
                    (uint8_t)g,
                    0
                });
        }
        bool changed = true;
        detail::processAllDeductions(
            table,
            uf,
            deductions,
            changed);


        while (true)
        {
            changed = false;

            //
            // Saturation phase
            //
            for (int c = 0;
                c < (int)table.table.size();
                ++c)
            {
                if (uf.find(c) != c)
                    continue;

                for (const Relator& relator
                    : relators)
                {
                    changed |=
                        scanRelator(
                            table,
                            uf,
                            relator,
                            c,
                            deductions);
                }
            }

            detail::processAllDeductions(
                table,
                uf,
                deductions,
                changed);

            //
            // If saturation produced something,
            // continue before defining anything.
            //
            if (changed)
                continue;

            //
            // Completely finished
            //
            if (!detail::hasUndefinedEntries(table))
                break;

            //
            // Felsch step:
            // define exactly one new coset
            //
            defineFirstUndefinedEntry(
                table,
                uf,
                deductions);

            detail::processAllDeductions(
                table,
                uf,
                deductions,
                changed);
        }

        detail::compressTable(
            table,
            uf);

        return table;
    }

    void defineFirstUndefinedEntry(
        CosetTable& table,
        UnionFind& uf,
        std::queue<Deduction>& deductions)
    {
        int g;

        int c =
            firstUndefinedCoset(
                table,
                g);

        if (c < 0)
            return;

        int n =
            table.newCoset();

        uf.add();

        deductions.push(
            {
                c,
                (uint8_t)g,
                n
            });
    }

    int firstUndefinedCoset(
        const CosetTable& table,
        int& generator)
    {
        for (int c = 0;c < (int)table.table.size();++c)
        {
            for (int g = 0;g < table.generators;++g)
            {
                if (table.table[c][g] == -1)
                {
                    generator = g;
                    return c;
                }
            }
        }

        return -1;
    }
    void coincidence(
        CosetTable& table,
        UnionFind& uf,
        int a,
        int b)
    {
        std::queue<Deduction> empty;
        bool changed = false;
        detail::coincidence(table, uf, a, b, empty, changed);
    }


	///SCAN 


    ScanResult scanWord(
        CosetTable& table,
        UnionFind& uf,
        const Relator& relator,
        int startCoset)
    {
        const int n =
            (int)relator.word.size();

        ScanResult result;

        result.forwardPos = 0;
        result.backwardPos = n - 1;

        result.forwardCoset =
            uf.find(startCoset);

        result.backwardCoset =
            uf.find(startCoset);

        //
        // forward
        //
        while (result.forwardPos < n)
        {
            const uint8_t g =
                relator.word[result.forwardPos];

            int next =
                table.table[result.forwardCoset][g];

            if (next == -1)
                break;

            result.forwardCoset =
                uf.find(next);

            ++result.forwardPos;
        }

        //
        // backward
        //
        while (result.backwardPos >= 0)
        {
            const uint8_t g =
                relator.word[result.backwardPos];

            int next =
                table.table[result.backwardCoset][g];

            if (next == -1)
                break;

            result.backwardCoset =
                uf.find(next);

            --result.backwardPos;
        }

        return result;
    }

    bool scanRelator(
        CosetTable& table,
        UnionFind& uf,
        const Relator& relator,
        int startCoset,
        std::queue<Deduction>& deductions)
    {
        bool changed = false;

        ScanResult S =
            scanWord(
                table,
                uf,
                relator,
                startCoset);

        const int n =
            (int)relator.word.size();

        //
        // complete scan
        //
        if (S.forwardPos == n &&
            S.backwardPos == -1)
        {
            if (uf.find(S.forwardCoset) !=
                uf.find(startCoset))
            {
                coincidence(
                    table,
                    uf,
                    S.forwardCoset,
                    startCoset);

                changed = true;
            }

            return changed;
        }

        //
        // exactly one missing edge
        //
        if (S.backwardPos ==
            S.forwardPos)
        {
            Deduction d;

            d.source =
                S.forwardCoset;

            d.generator =
                relator.word[S.forwardPos];

            d.target =
                S.backwardCoset;

            deductions.push(d);

            detail::processAllDeductions(
                table,
                uf,
                deductions,
                changed);

            return true;
        }

        //
        // scans crossed
        //
        if (S.forwardPos >
            S.backwardPos)
        {
            if (uf.find(S.forwardCoset) !=
                uf.find(S.backwardCoset))
            {
                coincidence(
                    table,
                    uf,
                    S.forwardCoset,
                    S.backwardCoset);

                changed = true;
            }

            return changed;
        }

        //
        // gap > 1
        //
        // do nothing
        //
        return changed;
    }


    CosetBFSData buildCosetBFSData(const CosetTable& table)
    {
        if (table.table.empty())
            throw std::runtime_error("buildCosetBFSData: empty coset table");

        const int cosetCount = static_cast<int>(table.table.size());
        const int genCount = table.generators;

        CosetBFSData data;
        data.parentCoset.assign(cosetCount, -1);
        data.parentGenerator.assign(cosetCount, -1);
        data.representativeWord.resize(cosetCount);

        std::vector<bool> visited(cosetCount, false);
        std::queue<int> q;

        // root coset
        visited[0] = true;
        q.push(0);

        while (!q.empty())
        {
            const int c = q.front();
            q.pop();

            data.order.push_back(c);

            for (int g = 0; g < genCount; ++g)
            {
                const int n = table.table[c][g];
                if (n < 0 || n >= cosetCount)
                    throw std::runtime_error("buildCosetBFSData: invalid table entry");

                if (!visited[n])
                {
                    visited[n] = true;
                    data.parentCoset[n] = c;
                    data.parentGenerator[n] = g;

                    data.representativeWord[n] = data.representativeWord[c];
                    data.representativeWord[n].push_back(static_cast<uint8_t>(g));

                    q.push(n);
                }
            }
        }

        return data;
    }


} // namespace Coxeter