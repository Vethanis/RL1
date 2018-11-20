#define _CRT_SECURE_NO_WARNINGS

#include "buffer.h"

#include "sokol_gfx.h"
#include "macro.h"
#include "store.h"

namespace Buffers
{
    Store<Buffer, 64> ms_store;

    slot Load(const char* name)
    {
        char path[256] = {0};
        sprintf(path, "assets/%s.vb", name);
        FILE* file = fopen(path, "rb");
        Assert(file);

        uint32_t vertCount = 0;
        fread(&vertCount, sizeof(uint32_t), 1, file);

        Vertex* verts = (Vertex*)malloc(sizeof(Vertex) * vertCount);
        fread(verts, sizeof(Vertex), vertCount, file);
        fclose(file);

        slot s = Create(name, verts, vertCount);

        free(verts);

        return s;
    }
    void Save(
        const char*     name, 
        const Vertex*   vertices, 
        uint32_t        count)
    {
        char path[256] = {0};
        sprintf(path, "assets/%s.vb", name);
        FILE* file = fopen(path, "wb");
        Assert(file);
        fwrite(&count, sizeof(uint32_t), 1, file);
        fwrite(vertices, sizeof(Vertex), count, file);
        fclose(file);
    }
    slot Create(
        const char*     name, 
        const Vertex*   vertices, 
        uint32_t        vertCount)
    {
        sg_buffer_desc desc = {0};
        desc.content = vertices;
        desc.size = sizeof(Vertex) * vertCount;
        slot s = ms_store.Create(name);
        Buffer* p = ms_store.Get(s);
        p->m_id = sg_make_buffer(&desc);
        p->m_count = vertCount;
        Assert(p->m_id.id != SG_INVALID_ID);
        return s;
    }
    void Destroy(slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        Buffer* p = ms_store.Get(s);
        sg_destroy_buffer(p->m_id);
        ms_store.DestroyUnchecked(s);
    }
    const Buffer* Get(slot s)
    {
        return ms_store.Get(s);
    }
    bool Exists(slot s)
    {
        return ms_store.Exists(s);
    }
    bool Exists(const char* name)
    {
        return ms_store.Exists(name);
    }
    slot Find(const char* name)
    {
        return ms_store.Find(name);
    }
    slot Find(uint64_t hash)
    {
        return ms_store.Find(hash);
    }
};
