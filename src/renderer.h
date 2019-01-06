#pragma once

#include <stdint.h>

#include "shaders/textured.h"
#include "shaders/flat.h"

namespace Renderer
{
    enum BufferType
    {
        Vertices = 0,
        Indices,
        BufferTypeCount,
    };
    enum Format
    {
        RGBA8 = 0,
        RGBA16F,
        RGBA32F,
        R32F,
        RG32F,
        RGB32F,
        FormatCount,
    };
    enum ImageType
    {
        Texture1D = 0,
        Texture2D,
        Texture3D,
        TextureCube,
        ImageTypeCount,
    };
    enum FilterType
    {
        Nearest = 0,
        Linear,
        LinearMipmap,
        FilterTypeCount,
    };
    enum WrapType
    {
        Clamp = 0,
        Repeat,
    };
    struct Buffer
    {
        uint32_t id;
        uint32_t count;
    };
    struct Texture
    {
        uint32_t id;
    };
    struct BufferDesc
    {
        void*           vertexData;
        void*           indexData;
        uint32_t        vertexBytes;
        uint32_t        indexBytes;
        uint32_t        elementCount;
    };
    struct TextureDesc
    {
        void*       data;
        uint32_t    width; 
        uint32_t    height;
        uint32_t    layers;
        ImageType   type;
        Format      format;
        FilterType  minFilter;
        FilterType  magFilter;
        WrapType    wrapType;
    };

    void Init();
    void Shutdown();
    void Begin();
    void End();
    void SetViewport(const vec4& viewport);
    void DrawBackground(const mat4& projection, const mat4& view);
    void DrawTextured(
        Buffer      buffer,
        Texture     mat,
        Texture     norm,
        const Textured::VSUniform& vsuni,
        const Textured::FSUniform& fsuni);
    void DrawFlat(
        Buffer buffer,
        const Flat::VSUniform& vsuni,
        const Flat::FSUniform& fsuni);
    Buffer CreateBuffer(const BufferDesc& desc);
    void DestroyBuffer(Buffer buffer);
    Texture CreateTexture(const TextureDesc& desc);
    void DestroyTexture(Texture texture);
};
