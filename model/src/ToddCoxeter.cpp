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


    namespace detail
    {

        static void rawLink(CosetTable& table, int a, uint8_t g, int b)
        {
            table.table[a][g] = b;
            table.table[b][g] = a;
        }


        static void processDeduction(
            CosetTable& table,
            const Deduction& d,
            std::queue<Deduction>& deductions,
            bool& changed)
        {
            int source = d.source;
            int target = d.target;
            const uint8_t g = d.generator;


            if (table.table[source][g] == -1)
            {
                rawLink(table, source, g, target);
                changed = true;
                return;
            }

            //coincidence(table, uf, existing, target, deductions, changed);
        }

        static void processAllDeductions(
            CosetTable& table,
            std::queue<Deduction>& deductions,
            bool& changed)
        {
            while (!deductions.empty())
            {
                Deduction d = deductions.front();
                deductions.pop();

                processDeduction(table, d, deductions, changed);
            }
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
        std::queue<Deduction> deductions;

        table.newCoset();

        // fill relations of the stabilizer
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
            deductions,
            changed);
        ////


        while (true)
        {
            //
            // Saturation phase
            //
            for (int c = 0;
                c < (int)table.table.size();
                ++c)
            {
                for (const Relator& relator
                    : relators)
                {
                    scanRelator(
                            table,
                            relator,
                            c,
                            deductions);
                }
            }

            //
            // Completely finished
            //
            if (!detail::hasUndefinedEntries(table))
                break;

        }

        return table;
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


	///SCAN 


    ScanResult scanWord(
        CosetTable& table,
        const Relator& relator,
        int startCoset)
    {
        const int n =
            (int)relator.word.size();

        ScanResult result;

        result.forwardPos = 0;
        result.backwardPos = n - 1;

        result.forwardCoset = startCoset;

        result.backwardCoset = startCoset;

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

            result.forwardCoset = next;

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

            result.backwardCoset = next;

            --result.backwardPos;
        }

        return result;
    }

    bool scanRelator(
        CosetTable& table,
        const Relator& relator,
        int startCoset,
        std::queue<Deduction>& deductions)
    {
        bool changed = false;

        ScanResult S =
            scanWord(
                table,
                relator,
                startCoset);

        const int n =
            (int)relator.word.size();



        //
        // one missing edge or more otherwise (scan crossed) dissmis
        //
        if (S.backwardPos == S.forwardPos || S.backwardPos - S.forwardPos >= 1)
        {
            Deduction d;

            d.source = S.forwardCoset;
            d.generator = relator.word[S.forwardPos];
            if (S.backwardPos - S.forwardPos >= 1)
            {
                d.target = table.table.size(); // generate new coset

                table.newCoset();

            }
            else {
                d.target = S.backwardCoset;
            }

            deductions.push(d);

            detail::processAllDeductions(
                table,
                deductions,
                changed);

            if (S.backwardPos - S.forwardPos >= 1) //continue until scan pass
            {
                scanRelator(table, relator, startCoset, deductions);
            }

            return true;
        }

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


    std::vector<std::vector<int>> edgesIndexFromTable(const CosetTable& vertexTable, const CosetBFSData& vertexBFSData,
        const CosetTable& edgeTable, const CosetBFSData& edgeBFSData) 
    {
        
        std::vector<std::vector<int>> edgesIndex;
		edgesIndex.resize(edgeTable.table.size());

		std::vector<int> He0 = { vertexBFSData.order[0], vertexBFSData.order[1] }; // TODO templatize for higher dimensions
		for (int edgeCoset : edgeBFSData.order)
		{
            std::vector<uint8_t> generator = edgeBFSData.representativeWord[edgeCoset];

            for (int vertexCoset : He0)
            {
				for (uint8_t g : generator)
				{
					vertexCoset = vertexTable.table[vertexCoset][g];
				}
				edgesIndex[edgeCoset].push_back(vertexCoset);
            }

		}
		return edgesIndex;
    }

} // namespace Coxeter