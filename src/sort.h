#pragma once

#include "lang.h"

template<typename T, typename C>
inline void Sort(T* x, i32 len, C cmp)
{
    if(len < 2)
    {
        return;
    }

    i32 i = 0;
    i32 j = len - 1;
    {
        let p = x[len / 2];
        while(i < j)
        {
            while(cmp(x[i], p) < 0)
            {
                ++i;
            }
            while(cmp(x[j], p) > 0)
            {
                --j;
            }

            if(i >= j)
            {
                break;
            }

            let t = x[i];
            x[i] = x[j];
            x[j] = t;

            ++i;
            --j;
        }
    }

    Sort(x, i, cmp);
    Sort(x + i, len - i, cmp);
}
