#define _CRT_SECURE_NO_WARNINGS

#include "image.h"
#include "macro.h"
#include "gen_array.h"

#include "sokol_gfx.h"
#include "stb_image.h"

namespace Images
{
    gen_array<sg_image> ms_store;

    slot Create(const char* name)
    {
        char path[256] = {0};
        sprintf(path, "assets/images/%s.png", name);
        int32_t width = 0;
        int32_t height = 0;
        void* data = stbi_load(path, &width, &height, nullptr, 4);
        Assert(data);

        sg_image_desc desc = {0};
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        desc.content.subimage[0][0].ptr = data;
        desc.content.subimage[0][0].size = sizeof(uint8_t) * 4u * width * height;
        desc.width = width;
        desc.height = height;
        sg_image img;
        img = sg_make_image(&desc);
        Assert(img.id != SG_INVALID_ID);
        stbi_image_free(data);

        slot s = ms_store.Create();
        ms_store.GetUnchecked(s) = img;

        return s;
    }
    slot Create(const Image& data)
    {
        sg_image_desc desc = {0};
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        desc.content.subimage[0][0].ptr = data.data;
        desc.content.subimage[0][0].size = sizeof(uint8_t) * 4u * data.width * data.height;
        desc.width = data.width;
        desc.height = data.height;

        sg_image img;
        img = sg_make_image(&desc);
        Assert(img.id != SG_INVALID_ID);

        slot s = ms_store.Create();
        ms_store.GetUnchecked(s) = img;

        return s;
    }
    void Destroy(slot s)
    {
        if(ms_store.Exists(s))
        {
            sg_image img = ms_store.GetUnchecked(s);
            sg_destroy_image(img);
            ms_store.DestroyUnchecked(s);
        }
    }
    const sg_image* Get(slot s)
    {
        return ms_store.Get(s);
    }
};
