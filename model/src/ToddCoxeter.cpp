#include "ToddCoxeter.h"

#include <queue>
#include <algorithm>

namespace Coxeter
{

    // =====================================================
    // CosetTable
    // =====================================================

    int CosetTable::newCoset()
    {
        table.emplace_back(
            generators,
            -1);

        return static_cast<int>(table.size()) - 1;
    }

    // =====================================================
    // UnionFind
    // =====================================================

    int UnionFind::add()
    {
        const int id =
            static_cast<int>(parent.size());

        parent.push_back(id);

        return id;
    }

    int UnionFind::find(int x)
    {
        if (parent[x] == x)
            return x;

        parent[x] =
            find(parent[x]);

        return parent[x];
    }

    void UnionFind::unite(
        int a,
        int b)
    {
        a = find(a);
        b = find(b);

        if (a != b)
            parent[b] = a;
    }

    // =====================================================
    // Internal Helpers
    // =====================================================

    namespace
    {

        void link(
            CosetTable& table,
            int a,
            int g,
            int b)
        {
            table.table[a][g] = b;
            table.table[b][g] = a;
        }

        int defineCoset(
            CosetTable& table,
            UnionFind& uf,
            std::queue<Deduction>& deductions,
            int coset,
            int generator)
        {
            const int newCoset =
                table.newCoset();

            uf.add();

            link(
                table,
                coset,
                generator,
                newCoset);

            deductions.push(
                { coset,generator });

            deductions.push(
                { newCoset,generator });

            return newCoset;
        }

        void processDeduction(
            CosetTable&,
            UnionFind&,
            std::queue<Deduction>&)
        {
            //
            // TODO:
            //
            // Real Todd-Coxeter deduction engine.
            //
            // Left intentionally empty for now.
            //
        }

        void scanRelator(
            CosetTable& table,
            UnionFind& uf,
            std::queue<Deduction>& deductions,
            const Relator& relator,
            int startCoset)
        {
            int current =
                uf.find(startCoset);

            for (uint8_t generator : relator.word)
            {
                current =
                    uf.find(current);

                int next =
                    table.table[current][generator];

                if (next == -1)
                {
                    next =
                        defineCoset(
                            table,
                            uf,
                            deductions,
                            current,
                            generator);
                }

                current = next;
            }

            coincidence(
                table,
                uf,
                current,
                startCoset);
        }

        void compressTable(
            CosetTable& table,
            UnionFind& uf)
        {
            std::vector<int> remap(
                table.table.size(),
                -1);

            int nextId = 0;

            for (size_t i = 0;i < table.table.size();++i)
            {
                const int rep =
                    uf.find(static_cast<int>(i));

                if (remap[rep] == -1)
                {
                    remap[rep] = nextId++;
                }
            }

            std::vector<std::vector<int>> newTable(
                nextId,
                std::vector<int>(
                    table.generators,
                    -1));

            for (size_t i = 0;i < table.table.size();++i)
            {
                const int rep =
                    uf.find(static_cast<int>(i));

                const int newRow =
                    remap[rep];

                for (int g = 0;
                    g < table.generators;
                    ++g)
                {
                    const int target =
                        table.table[i][g];

                    if (target == -1)
                        continue;

                    const int targetRep =
                        uf.find(target);

                    newTable[newRow][g] =
                        remap[targetRep];
                }
            }

            table.table =
                std::move(newTable);
        }

    } // anonymous namespace

    // =====================================================
    // Coincidence
    // =====================================================

    void coincidence(
        CosetTable& table,
        UnionFind& uf,
        int a,
        int b)
    {
        std::queue<std::pair<int, int>> Q;

        Q.push({ a,b });

        while (!Q.empty())
        {
            auto pair = Q.front();
            Q.pop();

            int x =
                uf.find(pair.first);

            int y =
                uf.find(pair.second);

            if (x == y)
                continue;

            uf.unite(x, y);

            const int rep =
                uf.find(x);

            for (int g = 0;
                g < table.generators;
                ++g)
            {
                int tx =
                    table.table[x][g];

                int ty =
                    table.table[y][g];

                if (tx == -1 &&
                    ty == -1)
                    continue;

                if (tx == -1)
                {
                    table.table[rep][g] =
                        ty;
                    continue;
                }

                if (ty == -1)
                {
                    table.table[rep][g] =
                        tx;
                    continue;
                }

                Q.push({ tx,ty });
            }
        }
    }

    // =====================================================
    // Enumeration
    // =====================================================

    CosetTable enumerateCosets(
        const Presentation& presentation,
        const std::vector<Relator>& relators,
        const std::vector<uint8_t>& subgroupGenerators)
    {
        CosetTable table;

        table.generators =
            presentation.rank;

        UnionFind uf;

        std::queue<Deduction> deductions;

        table.newCoset();
        uf.add();

        //
        // subgroup generators
        //
        // H = < generators >
        //
        for (uint8_t g : subgroupGenerators)
        {
            link(
                table,
                0,
                g,
                0);
        }

        bool changed = true;

        while (changed)
        {
            changed = false;

            const size_t before =
                table.table.size();

            const size_t cosetCount =
                table.table.size();

            for (size_t c = 0;
                c < cosetCount;
                ++c)
            {
                for (const Relator& relator
                    : relators)
                {
                    scanRelator(
                        table,
                        uf,
                        deductions,
                        relator,
                        static_cast<int>(c));
                }
            }

            while (!deductions.empty())
            {
                processDeduction(
                    table,
                    uf,
                    deductions);

                deductions.pop();
            }

            if (table.table.size() != before)
                changed = true;
        }

        compressTable(
            table,
            uf);

        return table;
    }

}