#define _CRT_SECURE_NO_WARNINGS

#include "image.h"

#include "name.h"
#include "macro.h"

#include "sokol_gfx.h"
#include "stb_image.h"


namespace Images
{
    Store<sg_image, 64> ms_store;

    slot Load(const char* name)
    {
        char path[256] = {0};
        sprintf(path, "assets/%s.png", name);

        int32_t width = 0;
        int32_t height = 0;
        void* data = stbi_load(path, &width, &height, nullptr, 4);

        Assert(data != nullptr);

        slot s = Create(name, data, width, height);

        stbi_image_free(data);

        return s;
    }
    slot Create(const char* name, const void* data, int32_t width, int32_t height)
    {
        sg_image_desc desc = {0};
        desc.content.subimage[0][0].ptr = data;
        desc.content.subimage[0][0].size = sizeof(uint8_t) * 4u * width * height;
        desc.width = width;
        desc.height = height;
        slot s = ms_store.Create(name);
        sg_image* p = ms_store.Get(s);
        *p = sg_make_image(&desc);
        Assert(p->id != SG_INVALID_ID);

        return s;
    }
    void Destroy(slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        sg_image* p = ms_store.Get(s);
        sg_destroy_image(*p);
        ms_store.DestroyUnchecked(s);
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
    slot Find(uint64_t hash)
    {
        return ms_store.Find(hash);
    }
};
