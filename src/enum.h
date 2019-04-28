#pragma once

#include "lang.h"

#define ENUM_VAL(name, expr)  name expr,
#define ENUM_STR(name, expr)  case name: return #name;
#define ENUM_CMP(name, expr)  if(strcmp(x, #name) == 0) { return name; }

#define ENUM_DECL(T, E)     \
    enum T { E(ENUM_VAL) }; \
    cstr Get##T##Str(T x);  \
    T Get##T##Val(cstr x);

#define ENUM_DEF(T, E)      \
    cstr Get##T##Str(T x) { \
        switch(x) { default: return NULL; E(ENUM_STR); } } \
    T Get##T##Val(cstr x) { E(ENUM_CMP); return (T)0; }

// #define EXAMPLE_ENUM(X) \
//     X(Big,)             \
//     X(Small,)           \
//     X(Tall,=5)

// ENUM_DECL(ExamleType, EXAMPLE_ENUM)
