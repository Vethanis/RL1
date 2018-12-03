#pragma once

#define PLM_ENABLE 0
#define ASSERT_TYPE 1

#define NELEM(x) ( sizeof(x) / (sizeof((x)[0])) )
#define Min(a, b) ( (a) < (b) ? (a) : (b) )
#define Max(a, b) ( (a) > (b) ? (a) : (b) )
#define Clamp(x, lo, hi) ( Min(hi, Max(lo, x)) )
#define Lerp(a, b, i) ( (a) + (i) * ((b) - (a)) )

#if PLM_ENABLE
    #include <stdio.h>
    #define PLM() { printf("%s %d\n", __FILE__, __LINE__); }
#endif // PLM_ENABLE

#if ASSERT_TYPE == 3
    #include <assert.h>
    #define Assert(x) assert(x)
#elif ASSERT_TYPE == 2
    #include <stdio.h>
    #define Assert(x) { if(!(x)){ printf("%s failed %s %i\n", #x, __FILE__, __LINE__); __debugbreak(); }}
#elif ASSERT_TYPE == 1
    #define Assert(x) { if(!(x)) { __debugbreak(); } }
#else
    #define Assert(x) 
#endif // ASSERT_TYPE
