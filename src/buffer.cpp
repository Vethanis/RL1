#include "buffer.h"

#include "sokol_gfx.h"
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

void BufferResource::Load() 
{
    const char* path = Names::Get(NS_Buffer)[m_name];
    Assert(path);
    switch(m_type)
    {
        case BT_File:
        {
            m_verts = ReadBuffer(path, m_count);
        }
        break;
        case BT_Procedural:
        {
            m_verts = GenerateBuffer(path, m_count);
        }
        break;
    }
    Assert(m_verts);
}

void BufferResource::Free()
{
    Assert(m_verts);
    free(m_verts);
    m_verts = nullptr;
}

void BufferResource::Init()
{
    sg_buffer_desc desc = {0};
    desc.content = m_verts;
    desc.size = sizeof(float) * m_count;
    m_id = sg_make_buffer(&desc);
    Assert(m_id.id != SG_INVALID_ID);
}

void BufferResource::Shutdown()
{
    Assert(m_id.id != SG_INVALID_ID);
    sg_destroy_buffer(m_id);
    m_id.id = SG_INVALID_ID;
}
