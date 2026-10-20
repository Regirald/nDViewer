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
            const Deduction& d)
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

	bool isAlive(const CosetTable& table, int coset)
	{
		return table.cosetEquivalenceList[coset] == coset;
	}

    bool isDefined(const CosetTable& table, int coset, int rel)
    {
        return table.table[coset][rel] != -1;
    }
    void CosetTable::newCoset()
    {
        table.emplace_back(generators, -1);

        int n = static_cast<int>(table.size());

        cosetEquivalenceList.push_back(n);
    }

    
   

    int firstUndefinedCoset(
        const CosetTable& table,
        int& generator)
    {
        for (int c = 0; c < (int)table.table.size();++c)
        {
            for (int g = 0; g < table.generators;++g)
            {
                if (isDefined(table, c, g))
                {
                    generator = g;
                    return c;
                }
            }
        }

        return -1;
    }



	//COINCIDENCE
	int rootRepresentative(CosetTable& table, int coset)
	{
		int r = coset;
		while (!isAlive(table, r))
		{
			r = table.cosetEquivalenceList[r];
		}
		int m = coset;
		while (!isAlive(table, m))
		{
			int next = table.cosetEquivalenceList[m];
			table.cosetEquivalenceList[m] = r;
			m = next;
		}
		return r;
	}
    void merge(CosetTable& table, int cosetA, int cosetB)
    {
		int coseta = rootRepresentative(table, cosetA);
		int cosetb = rootRepresentative(table, cosetB);
        if (coseta != cosetb)
        {
            if (coseta > cosetb)
                std::swap(coseta, cosetb);
            table.cosetEquivalenceList[cosetb] = coseta;
            table.coincidenceQueue.push(cosetb);
        }
    }
	void processCoincidence(
		CosetTable& table,
		int cosetA,
		int cosetB)
	{
        int nbRelators = table.table[0].size();

        merge(table, cosetA, cosetB);
        while (!table.coincidenceQueue.empty())
        {
            int currentDeadCoset = table.coincidenceQueue.front();
            table.coincidenceQueue.pop();
            for (int rel = 0; rel < nbRelators; ++rel)
            {
				if (!isDefined(table, currentDeadCoset, rel))
					continue;
                int otherCoset = table.table[currentDeadCoset][rel];
                table.table[otherCoset][rel] = -1;

                int rootCurrentDeadCoset = rootRepresentative(table, currentDeadCoset);
                int rootOtherCoset = rootRepresentative(table, otherCoset);

                if (table.table[rootCurrentDeadCoset][rel] != -1)
                    merge(table, rootOtherCoset, table.table[rootCurrentDeadCoset][rel]);

                else if (table.table[rootOtherCoset][rel] != -1)
                    merge(table, rootCurrentDeadCoset, table.table[rootOtherCoset][rel]);
                else
                {
                    table.table[rootCurrentDeadCoset][rel] = rootOtherCoset;
                    table.table[rootOtherCoset][rel] = rootCurrentDeadCoset;
                }
            }

        }
    
	}


    // CLEAN
    void compress(CosetTable & table)
    {
        int n = static_cast<int>(table.table.size());
        std::vector<int> newIndex(n, -1);
        int newCosetCount = 0;
        for (int i = 0; i < n; ++i)
        {
            if (isAlive(table, i))
            {
                newIndex[i] = newCosetCount++;
            }
        }
        std::vector<std::vector<int>> newTable(newCosetCount, std::vector<int>(table.generators, -1));
        std::vector<int> newEquivalenceList(newCosetCount);
        for (int i = 0; i < n; ++i)
        {
            if (isAlive(table, i))
            {
                int newI = newIndex[i];
                newEquivalenceList[newI] = newI;
                for (int g = 0; g < table.generators; ++g)
                {
                    int target = table.table[i][g];
                    if (target != -1)
                    {
                        int newTarget = newIndex[rootRepresentative(table, target)];
                        newTable[newI][g] = newTarget;
                    }
                }
            }
        }
        table.table = std::move(newTable);
        table.cosetEquivalenceList = std::move(newEquivalenceList);
    }



	///SCAN 


    ScanResult scanWordForward(
        CosetTable& table,
        const Generator& relator,
        int startCoset)
    {
        const int n = (int)relator.word.size() - 1;

        ScanResult result;

        result.forwardPos = 0;
        result.backwardPos = n;
        result.forwardCoset = startCoset;
        result.backwardCoset = startCoset;

        while (result.forwardPos <= n)
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

        return result;
    }


    ScanResult scanWordBackward(
        ScanResult& result,
        CosetTable& table,
        const Generator& relator,
        int startCoset)
    {

        while (result.backwardPos >= result.forwardPos)
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

    void scanGenerator(
        CosetTable& table,
        const Generator& generator,
        int startCoset)
    {
        if (table.table.size() > 2000)
            return;

        const int n = (int)generator.word.size() - 1;

        ScanResult scanResultForward = scanWordForward(
                table,
                generator,
                startCoset);

        if (scanResultForward.forwardPos > n)
        {
		    if (scanResultForward.forwardCoset != scanResultForward.backwardCoset)
            {
				processCoincidence(
					table,
					scanResultForward.forwardCoset,
					scanResultForward.backwardCoset);
            }
            else
		    {
			    // generator fully satisfied, nothing to do
			    return;
		    }
        }
        
		ScanResult scanResultFB = scanWordBackward(
            scanResultForward,
			table,
			generator,
			startCoset);


        if (scanResultFB.forwardPos > scanResultFB.backwardPos)
        {
			processCoincidence(
				table,
				scanResultFB.forwardCoset,
				scanResultFB.backwardCoset);
        }
        else if (scanResultFB.backwardPos == scanResultFB.forwardPos)
        {
            Deduction d;

            d.source = scanResultFB.forwardCoset;
            d.generator = generator.word[scanResultFB.forwardPos];
            d.target = scanResultFB.backwardCoset;

            detail::processDeduction(table, d);
        }
		else if (scanResultFB.backwardPos > scanResultFB.forwardPos)
		{
			Deduction d;
			d.source = scanResultFB.forwardCoset;
			d.generator = generator.word[scanResultFB.forwardPos];
			d.target = table.table.size(); // generate new coset
			table.newCoset();
            detail::processDeduction(table, d);

			scanGenerator(table, generator, startCoset);
		}
		else
		{
			// no deduction possible
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





    CosetTable enumerateCosets(
        const Presentation& presentation,
        const std::vector<Generator>& generators,
        const Subgroup& subgroup)
    {
        CosetTable table;
        table.generators = presentation.rank;
        table.cosetEquivalenceList.push_back(0);  //initially we only have the 0-th coset H
        table.newCoset();

        std::vector<int> relators;
        for (int i = 0; i < presentation.rank; ++i)
        {
            relators.push_back(i);
        }
        // fill relations of the stabilizer
        for (int g : subgroup.generators)
        {
            Deduction d(0, static_cast<uint8_t>(g), 0);
            detail::processDeduction(table, d);
        }


        int currentCoset = 0;
        while (currentCoset < (int)table.table.size())
        {
            for (const Generator& generator : generators)
            {
                if (!isAlive(table, currentCoset))
                    break;
                scanGenerator(table, generator, currentCoset);
            }

            if (isAlive(table, currentCoset))
            {
                for (int relator : relators)
                {
                    if (!isDefined(table, currentCoset, relator))
                    {
                        table.newCoset();
                        int newCoset = static_cast<int>(table.table.size()) - 1;
                        detail::rawLink(table, currentCoset, relator, newCoset);
                    }

                }
            }
            // try odd schlafli symbols
            if (table.table.size() > 2000)
            {
                detail::setUndefinedToZero(table);
                bool check = detail::hasUndefinedEntries(table);
                break;
            }

            if (!detail::hasUndefinedEntries(table))
                break;

            currentCoset++;
        }

        compress(table);
        return table;
    }

} // namespace Coxeter