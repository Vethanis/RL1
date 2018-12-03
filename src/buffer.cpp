#define _CRT_SECURE_NO_WARNINGS

#include "buffer.h"
#include <stdio.h>

#include "sokol_gfx.h"
#include "macro.h"
#include "store.h"
#include "hashstring.h"

static void loadFn(Buffer* p, Hash hash)
{
    const char* name = BufferString(hash).GetStr();
    Assert(name);
    BufferData data = Buffers::Load(name);
    *p = Buffers::Create(data);
    Buffers::Free(data);
}

static void destroyFn(Buffer* p)
{
    Buffers::Destroy(*p);
}

namespace Buffers
{
    Store<Buffer, 256, loadFn, destroyFn> ms_store;

    slot Load(BufferString name)
    {
        return ms_store.Create(name.GetStr());
    }
    void Destroy(slot s)
    {
        ms_store.Destroy(s);
    }
    void IncRef(slot s)
    {
        ms_store.IncRef(s);
    }
    void DecRef(slot s)
    {
        ms_store.DecRef(s);
    }
    const Buffer* Get(slot s)
    {
        return ms_store.Get(s);
    }
    bool Exists(slot s)
    {
        return ms_store.Exists(s);
    }
    slot Find(const char* name)
    {
        return ms_store.Find(name);
    }
    slot Find(Hash hash)
    {
        return ms_store.Find(hash);
    }

    bool CreateVB(const char* name)
    {
        // create a .vb file from a .obj
        char path[256] = {0};
        sprintf(path, "assets/models/%s.obj", name);
        FILE* file = fopen(path, "rb");
        Assert(file);
        {
            struct Face
            {
                int32_t p[3];
                int32_t n[3];
                int32_t u[3];
            };
            Array<vec3> positions;
            Array<vec3> normals;
            Array<vec2> uvs;
            Array<Face> faces;

            int32_t c = fgetc(file);
            while(c != EOF)
            {
                // comments
                if(c == '#')
                {

                }
                else if(c == 'v')
                {
                    int32_t d = fgetc(file);
                    switch(d)
                    {
                        // position
                        case ' ':
                        {
                            vec3 p;
                            int32_t n = fscanf(file, "%f %f %f", &p.x, &p.y, &p.z);
                            Assert(n == 3);
                            positions.grow() = p;
                        }
                        break;
                        // uv
                        case 't':
                        {
                            vec2 p;
                            int32_t n = fscanf(file, "%f %f", &p.x, &p.y);
                            Assert(n == 2);
                            uvs.grow() = p;
                        }
                        break;
                        case 'n':
                        {
                            vec3 p;
                            int32_t n = fscanf(file, "%f %f %f", &p.x, &p.y, &p.z);
                            Assert(n == 3);
                            normals.grow() = glm::normalize(p);
                        }
                        break;
                        default:
                        {
                            Assert(false);
                        }
                        break;
                    }
                }
                else if(c == 'f')
                {
                    Face f;
                    int32_t n = fscanf(file,
                        "%d/%d/%d %d/%d/%d %d/%d/%d",
                        f.p + 0, f.u + 0, f.n + 0,
                        f.p + 1, f.u + 1, f.n + 1,
                        f.p + 2, f.u + 2, f.n + 2);
                    Assert(n == 9);
                    faces.grow() = f;
                }
                // nextline
                while(c != EOF && c != '\n')
                {
                    c = fgetc(file);
                }
                c = fgetc(file);
            }
            fclose(file);

            Array<Vertex> verts;
            Array<uint32_t> inds;

            for(const Face& f : faces)
            {
                for(int32_t i = 0; i < 3; ++i)
                {
                    Vertex v;
                    v.position = positions[f.p[i] - 1];
                    v.normal = normals[f.n[i] - 1];
                    v.uv = uvs[f.u[i] - 1];
                    inds.grow() = (uint32_t)verts.findOrPush(v);
                }
            }
            BufferData bd;
            bd.vertices = verts.begin();
            bd.vertCount = verts.count();
            bd.indices = inds.begin();
            bd.indexCount = inds.count();
            Save(name, bd);
        }
        return true;
    }
    BufferData Load(const char* name)
    {
        BufferData out;

        char path[256] = {0};
        sprintf(path, "assets/buffers/%s.vb", name);
        FILE* file = fopen(path, "rb");

        if(!file)
        {
            bool created = CreateVB(name);
            Assert(created);
            file = fopen(path, "rb");
            Assert(file);
        }

        uint32_t vertCount = 0;
        uint32_t indexCount = 0;
        fread(&vertCount, sizeof(uint32_t), 1, file);
        fread(&indexCount, sizeof(uint32_t), 1, file);        
        
        Vertex* verts = (Vertex*)malloc(sizeof(Vertex) * vertCount);
        uint32_t* indices = (uint32_t*)malloc(sizeof(uint32_t) * indexCount);
        
        fread(verts, sizeof(Vertex), vertCount, file);
        fread(indices, sizeof(uint32_t), indexCount, file);
        fclose(file);

        out.vertCount = vertCount;
        out.indexCount = indexCount;
        out.vertices = verts;
        out.indices = indices;

        return out;
    }
    void Free(BufferData& data)
    {
        free(data.vertices);
        free(data.indices);
        memset(&data, 0, sizeof(BufferData));
    }
    void Save(const char* name, const BufferData& data)
    {
        char path[256] = {0};
        sprintf(path, "assets/buffers/%s.vb", name);
        FILE* file = fopen(path, "wb");
        Assert(file);
        fwrite(&data.vertCount, sizeof(data.vertCount), 1, file);
        fwrite(&data.indexCount, sizeof(data.indexCount), 1, file);
        fwrite(data.vertices, sizeof(data.vertices[0]), data.vertCount, file);
        fwrite(data.indices, sizeof(data.indices[0]), data.indexCount, file);
        fclose(file);
    }
    Buffer Create(const BufferData& data)
    {
        Buffer out;
        sg_buffer_desc  desc    = {0};

        desc.type       = SG_BUFFERTYPE_VERTEXBUFFER;
        desc.content    = data.vertices;
        desc.size       = sizeof(data.vertices[0]) * data.vertCount;
        out.m_vertices  = sg_make_buffer(&desc);
        Assert(out.m_vertices.id != SG_INVALID_ID);

        desc.type       = SG_BUFFERTYPE_INDEXBUFFER;
        desc.content    = data.indices;
        desc.size       = sizeof(data.indices[0]) * data.indexCount;
        out.m_indices   = sg_make_buffer(&desc);
        out.m_count     = data.indexCount;
        Assert(out.m_indices.id != SG_INVALID_ID);

        return out;
    }
    void Destroy(Buffer& buf)
    {
        sg_destroy_buffer(buf.m_vertices);
        sg_destroy_buffer(buf.m_indices);
        memset(&buf, 0, sizeof(Buffer));
    }
};
