#pragma once

#include "slice.h"

template<typename T>
inline size_t SwapRemove(Slice<T> list, size_t i)
{
    Copy(list[i], list.back());
    return list.size() - 1;
}

template<typename T>
inline size_t ShiftRemove(Slice<T> list, size_t i)
{
    size_t len = list.size() - 1;
    Move(&list[i], &list[i + 1], len - i);
    return len;
}
