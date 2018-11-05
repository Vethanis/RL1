#pragma once

#include <stdio.h>

#define NELEM(x) ( sizeof(x) / (sizeof((x)[0])) )
#define Min(a, b) ( (a) < (b) ? (a) : (b) )
#define Max(a, b) ( (a) > (b) ? (a) : (b) )
#define Clamp(x, lo, hi) ( Min(hi, Max(lo, x)) )
#define Lerp(a, b, i) ( (a) + (i) * ((b) - (a)) )

#define HEAVY_ASSERTS 0

#if HEAVY_ASSERTS
    #define Assert(x) { if(!(x)){ printf("%s failed %s %i\n", #x, __FILE__, __LINE__); __debugbreak(); }}
#else 
    #define Assert(x) { if(!(x)) { __debugbreak(); } }
#endif // HEAVY_ASSERTS
