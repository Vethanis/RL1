#include "buffer.h"

#include "sokol_gfx.h"
#include "resourcebase.h"
#include "name.h"

float* ReadBuffer(const char* path, int32_t& count)
{
    FILE* file = nullptr;
    fopen_s(&file, path, "rb");
    Assert(file);
    int32_t numRead = (int32_t)fread(&count, sizeof(int32_t), 1, file);
    Assert(numRead == 1);
    float* data = (float*)malloc(sizeof(float) * count);
    Assert(data);
    numRead = (int32_t)fread(data, sizeof(float), count, file);
    Assert(numRead == count);
    fclose(file);
    return data;
}

float* GenerateBuffer(const char* seed, int32_t& count)
{
    count = 0;
    return nullptr;
}

void Buffer::Load(const ResMeta& meta, Buffer& x)
{
    const char* path = Names::Get(NS_Buffer)[meta.m_name];
    Assert(path);
    switch(meta.m_loader)
    {
        case BL_File:
        {
            x.verts = ReadBuffer(path, x.count);
        }
        break;
        case BL_Gen:
        {
            x.verts = GenerateBuffer(path, x.count);
        }
        break;
        default:
        {
            Assert(false);
        }
        break;
    }
    Assert(x.verts);
}

void Buffer::Free(const ResMeta& meta, Buffer& x)
{
    Assert(x.verts);
    free(x.verts);
    x.verts = nullptr;
    x.count = 0;
}

void Buffer::Init(const ResMeta& meta, Buffer& x)
{
    sg_buffer_desc desc = {0};
    desc.content = x.verts;
    desc.size = sizeof(float) * x.count;
    x.id = sg_make_buffer(&desc);
    Assert(x.id.id != SG_INVALID_ID);
}

void Buffer::Shutdown(const ResMeta& meta, Buffer& x)
{
    Assert(x.id.id != SG_INVALID_ID);
    sg_destroy_buffer(x.id);
    x.id.id = SG_INVALID_ID;
}
