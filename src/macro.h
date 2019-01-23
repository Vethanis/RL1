#pragma once

#define PLM_ENABLE      0
#define ASSERT_TYPE     1
#define DEBUG_GL        0
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

#if DEBUG_GL
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
#else
    #define DebugGL() 
#endif // DEBUG_GL

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

template<size_t capacity>
inline void Format(char (&x)[capacity], const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(x, capacity, fmt, args);
    va_end(args);
    x[capacity - 1u] = 0;
}
