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
            std::queue<Deduction>& deductions)
        {
            int source = d.source;
            int target = d.target;
            const uint8_t g = d.generator;


            if (table.table[source][g] == -1)
            {
                rawLink(table, source, g, target);
                return;
            }

        }

        static void processAllDeductions(
            CosetTable& table,
            std::queue<Deduction>& deductions)
        {
            while (!deductions.empty())
            {
                Deduction d = deductions.front();
                deductions.pop();

                processDeduction(table, d, deductions);
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

        static void setUndefinedToZero(CosetTable& table)
        {
            for (auto& row : table.table)
            {
                for (int& v : row)
                {
                    if (v == -1)
                        v = 0;
                }
            }
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
            deductions);
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
            // try odd schlafli symbols
            if (table.table.size() > 100)
            {
                detail::setUndefinedToZero(table);
				bool check = detail::hasUndefinedEntries(table);
                break;
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

    void scanRelator(
        CosetTable& table,
        const Relator& relator,
        int startCoset,
        std::queue<Deduction>& deductions)
    {
        if (table.table.size() > 100)
            return;

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
                deductions);

            if (S.backwardPos - S.forwardPos >= 1) //continue until scan pass
            {
                scanRelator(table, relator, startCoset, deductions);
            }

        }

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
				//if (n == -1)
				//	continue;  //uncomment to try odd schlalfi symbols
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


    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> edgesAndFacesIndexFromTables(const CosetTable& vertexTable, std::vector<CosetBFSData> costetsBFSData, const std::vector<int>& schlafli)
    {
        
        std::vector<std::vector<int>> edgesIndex;
        std::vector<std::vector<int>> facesIndex;

        CosetBFSData vertexBFSData = costetsBFSData[0];
        CosetBFSData edgeBFSData = costetsBFSData[1];
		edgesIndex.resize(edgeBFSData.order.size());
        CosetBFSData faceBFSData;
		bool hasFaceData = costetsBFSData.size() > 2;
        if (hasFaceData)
        {
            faceBFSData = costetsBFSData[2];
		    facesIndex.resize(faceBFSData.order.size());
        }
        else
        {
            std::vector<int> face;
            for (int i = 0; i < schlafli[0]; ++i)
            {
                face.push_back({ vertexBFSData.order[i] });
            }
            facesIndex.push_back(face);
        }


		std::vector<int> He0 = { 0, 1};
        std::vector<int> Hf0;
		for (int i = 0; i < schlafli[0]; ++i)
		{
			Hf0.push_back(i);
		}

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
        if (hasFaceData)
        {
            for (int faceCoset : faceBFSData.order)
            {
                std::vector<uint8_t> generator = faceBFSData.representativeWord[faceCoset];
                for (int vertexCoset : Hf0)
                {
                    for (uint8_t g : generator)
                    {
                        vertexCoset = vertexTable.table[vertexCoset][g];
                    }
                    facesIndex[faceCoset].push_back(vertexCoset);
                }
            }
        }
		return std::make_pair(edgesIndex, facesIndex);
    }

    std::vector<std::vector<std::vector<int>>> edgesIndexFromTable(const std::vector<CosetTable>& Tables, const std::vector<CosetBFSData>& costetsBFSData)
    {

        std::vector<std::vector<int>> elements0;

        for (int i = 0; i < Tables.size(); i++)
        {

        }
        return std::vector<std::vector<std::vector<int>>>();
    }

} // namespace Coxeter