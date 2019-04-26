#pragma once

#include "slice.h"

template<typename T>
inline usize SwapRemove(Slice<T> list, usize i)
{
    Copy(list[i], list.back());
    return list.size() - 1;
}

template<typename T>
inline usize ShiftRemove(Slice<T> list, usize i)
{
    usize len = list.size() - 1;
    Move(&list[i], &list[i + 1], len - i);
    return len;
}
