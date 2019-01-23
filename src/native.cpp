#include "native.h"
#include "dict.h"

namespace Native
{
    using namespace ScriptTypes;

    static Array<NativeFuncData> ms_data;

    void Create(NativeID id, const NativeFuncData& data)
    {
        while(id.id >= ms_data.count())
        {
            MemZero(ms_data.grow());
        }
        ms_data[id.id] = data;
    }
    void Exec(NativeExecData data)
    {
        ms_data[data.id.id].func(data);
    }
    void Sleep(ResourceHandle handle)
    {
        ms_data[handle.src.id].sleeper(handle);
    }
    void Wake(ResourceHandle handle)
    {
        ms_data[handle.src.id].waker(handle);
    }
    void Destroy(ResourceHandle handle)
    {
        ms_data[handle.src.id].destructor(handle);
    }
};
