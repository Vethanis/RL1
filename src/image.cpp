#define _CRT_SECURE_NO_WARNINGS

#include "image.h"
#include "macro.h"
#include "gen_array.h"
#include "stb_image.h"

namespace Images
{
    static gen_array<Renderer::Texture> ms_store;

    slot Create(const char* name)
    {
        char path[MAX_PATH_LEN] = {0};
        Format(path, "assets/images/%s.png", name);
        int32_t width = 0;
        int32_t height = 0;
        void* data = stbi_load(path, &width, &height, nullptr, 4);
        Assert(data);

        Renderer::TextureDesc desc;
        desc.minFilter = Renderer::LinearMipmap;
        desc.magFilter = Renderer::Linear;
        desc.data = data;
        desc.width = width;
        desc.height = height;
        desc.layers = 1;
        desc.type = Renderer::Texture2D;
        desc.format = Renderer::RGBA8;
        desc.wrapType = Renderer::Repeat;

        slot s = Create(desc);
        stbi_image_free(data);

        return s;
    }
    slot Create(const Renderer::TextureDesc& desc)
    {
        Renderer::Texture texture = Renderer::CreateTexture(desc);
        Assert(texture.id != 0);

        slot s = ms_store.Create();
        ms_store.GetUnchecked(s) = texture;

        return s;
    }
    void Destroy(slot s)
    {
        if(ms_store.Exists(s))
        {
            Renderer::Texture texture = ms_store.GetUnchecked(s);
            Renderer::DestroyTexture(texture);
            ms_store.DestroyUnchecked(s);
        }
    }
    const Renderer::Texture* Get(slot s)
    {
        return ms_store.Get(s);
    }
};
