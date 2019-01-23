#pragma once

#include "scriptTypes.h"

namespace Compiler
{
    bool Compile(const char* src, ScriptTypes::CompileResult& result);
};
