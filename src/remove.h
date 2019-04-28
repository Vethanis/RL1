#pragma once

#include "lang.h"
#include "memory.h"

template<typename T>
inline usize SwapRemove(Slice<T> list, usize i)
{
    usize len = list.size() - 1;
    CopyR(list[i], list[len]);
    return len;
}

template<typename T>
inline usize ShiftRemove(Slice<T> list, usize i)
{
    usize len = list.size() - 1;
    MoveP(&list[i], &list[i + 1], len - i);
    return len;
}
