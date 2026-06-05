#include "CoxeterPresentation.h"

namespace Coxeter
{

    Presentation buildPresentation(
        const std::vector<int>& schlafli)
    {
        Presentation P;

        P.rank =
            static_cast<int>(schlafli.size()) + 1;

        P.M.assign(
            P.rank,
            std::vector<int>(P.rank, 2));

        for (int i = 0;i < P.rank;++i)
        {
            P.M[i][i] = 1;
        }

        for (size_t i = 0;i < schlafli.size();++i)
        {
            const int m = schlafli[i];

            P.M[i][i + 1] = m;
            P.M[i + 1][i] = m;
        }

        return P;
    }

    std::vector<Relator> buildRelators(
        const Presentation& P)
    {
        std::vector<Relator> relators;

        //
        // s_i² = 1
        //
        for (int i = 0;i < P.rank;++i)
        {
            Relator r;

            r.word.push_back(
                static_cast<uint8_t>(i));

            r.word.push_back(
                static_cast<uint8_t>(i));

            relators.push_back(
                std::move(r));
        }

        //
        // (s_i s_j)^m = 1
        //
        for (int i = 0;i < P.rank;++i)
        {
            for (int j = i + 1;j < P.rank;++j)
            {
                const int m = P.M[i][j];

                if (m <= 1)
                    continue;

                Relator r;

                for (int k = 0;k < m;++k)
                {
                    r.word.push_back(
                        static_cast<uint8_t>(i));

                    r.word.push_back(
                        static_cast<uint8_t>(j));
                }

                relators.push_back(
                    std::move(r));
            }
        }

        return relators;
    }

}