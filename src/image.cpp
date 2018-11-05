#include "image.h"

#include "resourcebase.h"
#include "name.h"
#include "macro.h"

#include "sokol_gfx.h"
#include "stb_image.h"

void Image::Load(const ResMeta& meta, Image& x)
{
    const char* path = Names::Get(NS_Image)[meta.m_name];
    Assert(path);
    x.data = stbi_load(path, &x.width, &x.height, nullptr, 4);
    Assert(x.data);
}

void Image::Free(const ResMeta& meta, Image& x)
{
    Assert(x.data);
    stbi_image_free(x.data);
    x.data = nullptr;
}

void Image::Init(const ResMeta& meta, Image& x)
{
    sg_image_desc desc = {0};
    desc.width = x.width;
    desc.height = x.height;
    desc.content.subimage[0][0].ptr = x.data;
    desc.content.subimage[0][0].size = x.width * x.height * 4 * sizeof(uint8_t);
    x.id = sg_make_image(&desc);
    Assert(x.id.id != SG_INVALID_ID);
}

void Image::Shutdown(const ResMeta& meta, Image& x)
{
    sg_destroy_image(x.id);
    x.id.id = SG_INVALID_ID;
}
