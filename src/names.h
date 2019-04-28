#pragma once

#include "lang.h"

def cstr NullName = "null";

namespace Names
{
    bool Add(u64 hash, cstr name);
    bool Remove(u64 hash);
    cstr Lookup(u64 hash);
};
