#pragma once

#include "lang.h"
#include "array.h"

struct Parser;
struct Scanner;

using ParseFn = u32(*)(const Parser* pc, Scanner* sc);
using RuleFn = bool(*)(char c);

struct Token
{
    cstr begin;
    cstr end;
    u32  id;
};

using Tokens = Array<Token>;

struct Scanner
{
    cstr   origin;
    cstr   cursor;
    Tokens tokens;
};

struct Parser
{
    const ParseFn fn;
    const u32     id;
    const void*   data;
};

enum MetaToken
{
    MetaToken_Null = 0,
    MetaToken_Some,
    MetaToken_None,

    MetaToken_Count
};
