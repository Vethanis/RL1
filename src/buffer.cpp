#define _CRT_SECURE_NO_WARNINGS

#include "buffer.h"

#include "sokol_gfx.h"
#include "gen_array.h"

namespace Buffers
{
    gen_array<Buffer> ms_store;

    slot Create(const BufferData& data)
    {
        Buffer out;
        sg_buffer_desc desc = {0};

        if(data.vertices)
        {
            desc.type       = SG_BUFFERTYPE_VERTEXBUFFER;
            desc.content    = data.vertices;
            desc.size       = data.size * data.vertCount;
            out.m_vertices  = sg_make_buffer(&desc);
            out.m_count     = data.vertCount;
            Assert(out.m_vertices.id != SG_INVALID_ID);
        }

        if(data.indices)
        {
            desc.type       = SG_BUFFERTYPE_INDEXBUFFER;
            desc.content    = data.indices;
            desc.size       = sizeof(data.indices[0]) * data.indexCount;
            out.m_indices   = sg_make_buffer(&desc);
            out.m_count     = data.indexCount;
            Assert(out.m_indices.id != SG_INVALID_ID);
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
            sg_destroy_buffer(buf.m_vertices);
            sg_destroy_buffer(buf.m_indices);
            ms_store.DestroyUnchecked(s);
        }
    }
    const Buffer* Get(slot s)
    {
        return ms_store.Get(s);
    }
};
