#pragma once

#include "swap.h"

template<typename T>
static void Sort(T* x, int32_t len)
{
    if(len <= 8)
    {
        for(int32_t i = 0; i < len; ++i)
        {
            int32_t c = i;
            for(int32_t j = i + 1; j < len; ++j)
            {
                if(x[j] < x[c])
                {
                    c = j;
                }
            }
            Swap(x[c], x[i]);
        }
        return;
    }

    int32_t i, j;
    {
        const int32_t p = len / 2;
        for(i = 0, j = len - 1; ; ++i, --j)
        {
            while(x[i] < x[p]) ++i;
            while(x[j] > x[p]) --j;

            if(i >= j) 
                break;

            Swap(x[i], x[j]);
        }
    }

    Sort(x, i);
    Sort(x + i, len - i);
}
