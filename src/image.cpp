#include "image.h"

#include "name.h"
#include "macro.h"

#include "sokol_gfx.h"
#include "stb_image.h"

void ImageResource::Load()
{
    const char* path = Names::Get(NS_Image)[m_name];
    Assert(path);
    m_data = stbi_load(path, &m_width, &m_height, nullptr, 4);
    Assert(m_data);
}

void ImageResource::Free()
{
    Assert(m_data);
    stbi_image_free(m_data);
    m_data = nullptr;
}

void ImageResource::Init()
{
    sg_image_desc desc = {0};
    desc.width = m_width;
    desc.height = m_height;
    desc.content.subimage[0][0].ptr = m_data;
    desc.content.subimage[0][0].size = m_width * m_height * 4 * sizeof(uint8_t);
    m_id = sg_make_image(&desc);
    Assert(m_id.id != SG_INVALID_ID);
}

void ImageResource::Shutdown()
{
    Assert(m_id.id != SG_INVALID_ID);
    sg_destroy_image(m_id);
    m_id.id = SG_INVALID_ID;
}

