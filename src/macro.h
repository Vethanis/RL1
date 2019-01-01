#pragma once

#define PLM_ENABLE 0
#define ASSERT_TYPE 1
#define DEBUG_GL 0

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