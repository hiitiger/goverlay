#pragma once

#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <ctime>
#include <time.h>
#include <cctype>
#include <locale>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <list>
#include <array>
#include <unordered_map>
#include <memory>
#include <numeric>
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <atomic>
#include <chrono>
#include <tuple>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <assert.h>
#include <experimental/filesystem>
#include <experimental/coroutine>

namespace Storm
{
    inline int round(double d)
    {
        return d >= 0.0 ? int(d + 0.5) : int(d - double(int(d - 1)) + 0.5) + int(d - 1);
    }

    inline int round(float d)
    {
        return d >= 0.0 ? int(d + 0.5f) : int(d - float(int(d - 1)) + 0.5f) + int(d - 1);
    }

    template <typename T>
    inline T abs(const T &t)
    {
        return t >= 0 ? t : -t;
    }

    inline bool fuzzyCompare(float d1, float d2)
    {
        return (abs(d1 - d2) * 100000.f <= std::min(abs(d1), abs(d2)));
    }

    inline bool fuzzyCompare(double d1, double d2)
    {
        return (abs(d1 - d2) * 1000000000000. <= std::min(abs(d1), abs(d2)));
    }

    inline bool fuzzyIsZero(float f)
    {
        return abs(f) <= 0.00001f;
    }

    inline bool fuzzyIsZero(double d)
    {
        return abs(d) <= 0.000000000001;
    }

    template<class T>
    inline bool isZero(T a)
    {
        return a == T();
    }

    template<class T>
    inline bool compareValue(T a1, T a2)
    {
        return a1 == a2;
    }

    template<>
    inline bool compareValue(float a1, float a2)
    {
        return fuzzyCompare(a1, a2);
    }

    template<>
    inline bool compareValue(double a1, double a2)
    {
        return fuzzyCompare(a1, a2);
    }

    template<class T, int n>
    inline void copyArray(std::array<T, n>& dst, const std::array<T, n>& src)
    {
        for (int i = 0; i != n; ++i)
        {
            dst[i] = src.at(i);
        }
    }
}

