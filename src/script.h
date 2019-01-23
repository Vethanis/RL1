#pragma once

#include "scriptTypes.h"

namespace Script
{
    ScriptTypes::ProcessHandle Create(const ScriptTypes::ProcessCreateInfo& info);
    void Destroy(ScriptTypes::ProcessHandle hdl);
    void Wake(ScriptTypes::ProcessHandle hdl);
    void Sleep(ScriptTypes::ProcessHandle hdl);
    void Run(ScriptTypes::ProcessHandle hdl, double usecs);
};
