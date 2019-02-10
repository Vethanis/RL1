#pragma once

#define PLM_ENABLE      0
#define ASSERT_TYPE     1
#define VK_BACKEND      1
#define GFX_DBG         1
#define MAX_PATH_LEN    256

#define NELEM(x) ( sizeof(x) / (sizeof((x)[0])) )

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

#if VK_BACKEND
    #define VK_ONLY(x) x
    #if GFX_DBG
        #define DVK_ONLY(x) x
    #else 
        #define DVK_ONLY(x) 
    #endif // GFX_DBG

    #define GL_ONLY(x) 
    #define DGL_ONLY(x) 
    #define DebugGL() 
#else 
    #define GL_ONLY(x) x
    #if GFX_DBG
        #define DebugGL() \
        {\
            const char* error = 0; \
            uint32_t value = glGetError(); \
            while(value != GL_NO_ERROR) \
            { \
                switch(value) \
                { \
                    case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break; \
                    case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break; \
                    case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break; \
                    case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break; \
                    case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break; \
                    case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break; \
                    case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break; \
                } \
                printf("GL ERROR: %s %d :: %s\n", __FILE__, __LINE__, error); \
                value = glGetError(); \
            } \
            Assert(!error); \
        }
        #define DGL_ONLY(x) x
    #else
        #define DebugGL() 
        #define DGL_ONLY(x) 
    #endif  // GFX_DBG

    #define VK_ONLY(x) 
    #define DVK_ONLY(x) 
#endif // VK_BACKEND

#include <string.h>
#include <stdarg.h>

template<typename T>
inline T Min(T a, T b)
{
    return a < b ? a : b;
}

template<typename T>
inline T Max(T a, T b)
{
    return a > b ? a : b;
}

template<typename T>
inline T Clamp(T x, T lo, T hi)
{
    return Min(hi, Max(lo, x));
}

template<typename T>
inline T Lerp(T a, T b, T i)
{
    return a + i * (b - a);
}

template<typename T>
inline void MemZero(T& x)
{
    memset(&x, 0, sizeof(T));
}

template<typename T, size_t capacity>
inline void MemZero(T (&x)[capacity])
{
    memset(x, 0, sizeof(T) * capacity);
}

template<typename T>
inline void MemZero(T* x, size_t count)
{
    memset(x, 0, sizeof(T) * count);
}

template<typename T>
inline void Copy(T& dst, const T& src)
{
    memcpy(&dst, &src, sizeof(T));
}

template<typename T>
inline void Copy(T* dst, const T* src, size_t count)
{
    memcpy(dst, src, sizeof(T) * count);
}

template<typename T>
inline void Assume(T& dst, T&& src)
{
    memcpy(&dst, &src, sizeof(T));
    memset(&src, 0, sizeof(T));
}

template<typename T>
inline void Assume(T& dst, T& src)
{
    memcpy(&dst, &src, sizeof(T));
    memset(&src, 0, sizeof(T));
}

template<int32_t capacity>
inline int32_t Format(char (&x)[capacity], const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int32_t stringLen = vsnprintf(x, capacity, fmt, args);
    va_end(args);
    Assert(stringLen >= 0);
    int32_t stringLen = Clamp(stringLen, 0, capacity - 1);
    x[stringLen] = 0;
    return stringLen;
}

inline int32_t strlcpy(char* dst, const char* src, int32_t len)
{
    --len;
    int32_t i = 0;
    for(; i < len && src[i]; ++i)
    {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return i;
}

template<int32_t capacity>
inline int32_t Copy(char (&dst)[capacity], const char* src)
{
    return strlcpy(dst, src, capacity);
}

template<typename T>
static void Swap(T& a, T& b)
{
    uint8_t mem[sizeof(T)];
    memcpy(mem, &a, sizeof(T));
    memcpy(&a, &b, sizeof(T));
    memcpy(&b, mem, sizeof(T));
}
