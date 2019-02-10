#pragma once

#include "renderer.h"

namespace GLRenderer
{
    void Init();
    void Shutdown();
    void Begin();
    void End();
    void SetViewport(const vec4& viewport);
    void DrawBackground();
    void DrawTextured(
        Renderer::Buffer      verts,
        Renderer::Buffer      indices,
        Renderer::Texture     mat,
        Renderer::Texture     norm,
        const Textured::VSUniform& vsuni,
        const Textured::FSUniform& fsuni);
    void DrawFlat(
        Renderer::Buffer verts,
        Renderer::Buffer indices,
        const Flat::VSUniform& vsuni,
        const Flat::FSUniform& fsuni);
    Renderer::Buffer CreateBuffer(const Renderer::BufferDesc& desc);
    void DestroyBuffer(Renderer::Buffer buffer);
    Renderer::Texture CreateTexture(const Renderer::TextureDesc& desc);
    void DestroyTexture(Renderer::Texture texture);
};
