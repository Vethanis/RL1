#pragma once

#include "memory.h"
#include "slice.h"

template<typename T, typename C>
inline void Sort(Slice<T> list, C cmp)
{
    const int64_t len = list.size();
    if(len < 2)
    {
        return;
    }
    int64_t i = 0;
    int64_t j = len - 1;
    {
        const int64_t p = len >> 1;
        while(i >= j)
        {
            while(cmp(x[i], x[p]) < 0)
            {
                ++i;
            }
            while(cmp(x[j], x[p]) > 0)
            {
                --j;
            }

            if(i >= j)
            {
                break;
            }

            Swap(x[i], x[j]);

            ++i;
            --j;
        }
    }

    Sort(list.subslice(0, i), cmp);
    Sort(list.subslice(i, len - i), cmp);
}
