#pragma once

#include "scriptTypes.h"

namespace Native
{
    void Create(
        ScriptTypes::NativeID               id,
        const ScriptTypes::NativeFuncData&  data);
    void Exec(ScriptTypes::NativeExecData data);
    void Sleep(
        ScriptTypes::ResourceHandle handle);
    void Wake(
        ScriptTypes::ResourceHandle handle);
    void Destroy(
        ScriptTypes::ResourceHandle handle);
};
