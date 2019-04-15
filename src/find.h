#pragma once

#include "slice.h"

template<typename T, typename C>
inline constexpr i64 Find(
    Slice<const T>  list,
    const T&        key,
    C               cmp)
{
    const i64 len = list.size();
    for (i64 i = 0; i < len; ++i)
    {
        if (cmp(list[i], key) == 0)
        {
            return i;
        }
    }
    return -1;
}

template<typename T, typename C>
inline constexpr i64 RFind(
    Slice<const T>  list,
    const T&        key,
    C               cmp)
{
    const i64 last = (i64)list.size() - 1;
    for (i64 i = last; i >= 0; --i)
    {
        if (cmp(list[i], key) == 0)
        {
            return i;
        }
    }
    return -1;
}
