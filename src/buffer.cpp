
#include "buffer.h"
#include "renderer.h"
#include "gen_array.h"
#include "macro.h"

namespace Buffers
{
    gen_array<Buffer> ms_store;

    slot Create(const BufferData& data)
    {
        Buffer out;
        Renderer::BufferDesc desc;

        if(data.vertices)
        {
            desc.data       = data.vertices;
            desc.count      = data.vertCount;
            desc.elementSize= data.size;
            desc.type       = Renderer::Vertices;
            out.m_vertices  = Renderer::CreateBuffer(desc);
            out.m_count     = data.vertCount;
            Assert(out.m_vertices.id != 0);
        }

        if(data.indices)
        {
            desc.data       = data.indices;
            desc.count      = data.indexCount;
            desc.elementSize= sizeof(data.indices[0]);
            desc.type       = Renderer::Indices;
            out.m_indices   = Renderer::CreateBuffer(desc);
            out.m_count     = data.indexCount;
            Assert(out.m_indices.id != 0);
        }

        slot s = ms_store.Create();
        ms_store.GetUnchecked(s) = out;
        return s;
    }
    void Destroy(slot s)
    {
        if(ms_store.Exists(s))
        {
            Buffer& buf = ms_store.GetUnchecked(s);
            Renderer::DestroyBuffer(buf.m_vertices);
            Renderer::DestroyBuffer(buf.m_indices);
            ms_store.DestroyUnchecked(s);
        }
    }
    const Buffer* Get(slot s)
    {
        return ms_store.Get(s);
    }
};
