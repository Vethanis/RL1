#pragma once

#include <stdint.h>
#include "slot.h"

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
    enum TextureType
    {
        Texture1D = 0,
        Texture2D,
        Texture3D,
        TextureCube,
        TextureTypeCount,
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
    struct Buffer  { slot id; };
    struct Texture { slot id; };
    struct BufferDesc
    {
        void*           data;
        uint32_t        stride; // size of each element, in bytes
        uint32_t        count;  // number of elements
        BufferType      type;
    };
    struct TextureDesc
    {
        void*       data;
        uint32_t    width; 
        uint32_t    height;
        uint32_t    layers;
        TextureType type;
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
    void DrawBackground();
    void DrawTextured(
        Buffer      verts,
        Buffer      indices,
        Texture     mat,
        Texture     norm,
        const Textured::VSUniform& vsuni,
        const Textured::FSUniform& fsuni);
    void DrawFlat(
        Buffer verts,
        Buffer indices,
        const Flat::VSUniform& vsuni,
        const Flat::FSUniform& fsuni);
    Buffer CreateBuffer(const BufferDesc& desc);
    void DestroyBuffer(Buffer buffer);
    Texture CreateTexture(const TextureDesc& desc);
    void DestroyTexture(Texture texture);
};
