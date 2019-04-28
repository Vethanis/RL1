#pragma once

#include "lang.h"

template<typename T>
static constexpr isize Find(Slice<const T> x, T key)
{
    for (isize i = 0; i < len; ++i)
    {
        if (x[i] == key)
        {
            return i;
        }
    }
    return -1;
}

template<typename T, typename C>
static constexpr isize Find(Slice<const T> x, T key, C cmp)
{
    for (isize i = 0; i < len; ++i)
    {
        if (cmp(x[i], key) == 0)
        {
            return i;
        }
    }
    return -1;
}

template<typename T>
static constexpr isize RFind(Slice<const T> x, T key)
{
    for (isize i = list.size() - 1; i >= 0; --i)
    {
        if (x[i] == key)
        {
            return i;
        }
    }
    return -1;
}

template<typename T, typename C>
static constexpr isize RFind(Slice<const T> x, T key, C cmp)
{
    for (isize i = list.size() - 1; i >= 0; --i)
    {
        if (cmp(x[i], key) == 0)
        {
            return i;
        }
    }
    return -1;
}

template<typename T>
static constexpr bool Exists(Slice<const T> x, T key)
{
    for (usize i = 0; i < x.size(); ++i)
    {
        if (x[i] == key)
        {
            return true;
        }
    }
    return false;
}

template<typename T, typename C>
static constexpr bool Exists(Slice<const T> x, T key, C cmp)
{
    for (usize i = 0; i < x.size(); ++i)
    {
        if (cmp(x[i], key) == 0)
        {
            return true;
        }
    }
    return false;
}
