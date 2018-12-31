
#include "buffer.h"
#include "renderer.h"
#include "gen_array.h"
#include "macro.h"

namespace Buffers
{
    gen_array<Renderer::Buffer> ms_store;

    slot Create(const Renderer::BufferDesc& desc)
    {
        Renderer::Buffer buffer = Renderer::CreateBuffer(desc);
        slot s = ms_store.Create();
        ms_store.GetUnchecked(s) = buffer;
        return s;
    }
    void Destroy(slot s)
    {
        if(ms_store.Exists(s))
        {
            Renderer::Buffer& buffer = ms_store.GetUnchecked(s);
            Renderer::DestroyBuffer(buffer);
            ms_store.DestroyUnchecked(s);
        }
    }
    const Renderer::Buffer* Get(slot s)
    {
        return ms_store.Get(s);
    }
};
