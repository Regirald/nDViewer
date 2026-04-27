#pragma once

template <size_t Dim, size_t CurrentDim = Dim>
struct GridLooper
{
    template <typename Func>
    static void loop(const Vec<Dim, int>& size, Func&& f)
    {
        Vec<Dim, int> index{};
        processingLoop(size, index, std::forward<Func>(f));
    }

    template <typename Func>
    static void processingLoop(const Vec<Dim, int>& size, Vec<Dim, int>& currentIndex, Func&& f)
    {
        for (currentIndex[CurrentDim - 1] = 0;
            currentIndex[CurrentDim - 1] < size[CurrentDim - 1];
            ++currentIndex[CurrentDim - 1])
        {
            GridLooper<Dim, CurrentDim - 1>::processingLoop(size, currentIndex, f);
        }
    }
};


template <size_t Dim>
struct GridLooper<Dim, 0>
{
    template <typename Func>
    static void processingLoop(const Vec<Dim, int>& size, Vec<Dim, int>& currentIndex, Func&& f)
    {
        f(currentIndex);
    }
};
