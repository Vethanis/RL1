#define _CRT_SECURE_NO_WARNINGS

#include "image.h"

#include "name.h"
#include "macro.h"

#include "sokol_gfx.h"
#include "stb_image.h"

void loadFn(sg_image* img, Hash hash)
{
    const char* name = ImageString(hash).GetStr();
    Assert(name);
    Image data = Images::Load(name);
    *img = Images::Create(data);
    Images::Free(data);
}

void destroyFn(sg_image* img)
{
    sg_destroy_image(*img);
    img->id = SG_INVALID_ID;
}

namespace Images
{
    Store<sg_image, 256, loadFn, destroyFn> ms_store;

    slot Load(ImageString name)
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
    const sg_image* Get(slot s)
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
    slot Find(Hash hash)
    {
        return ms_store.Find(hash);
    }

    Image Load(const char* name)
    {
        char path[256] = {0};
        sprintf(path, "assets/images/%s.png", name);
        int32_t width = 0;
        int32_t height = 0;
        void* data = stbi_load(path, &width, &height, nullptr, 4);
        Assert(data);
        Image img;
        img.data = data;
        img.width = width;
        img.height = height;
        return img;
    }
    void Free(Image& img)
    {
        stbi_image_free(img.data);
        memset(&img, 0, sizeof(Image));
    }
    sg_image Create(const Image& data)
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
        return img;
    }
    void Destroy(sg_image img)
    {
        sg_destroy_image(img);
    }
};
