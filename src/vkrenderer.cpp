#include "vkrenderer.h"

namespace VkRenderer
{
    void Init()
    {

    }
    void Shutdown()
    {

    }
    void Begin()
    {
        
    }
    void End()
    {
        
    }
    void SetViewport(const vec4& viewport)
    {
        
    }
    void DrawBackground(const mat4& projection, const mat4& view)
    {
        
    }
    void DrawTextured(
        Buffer      buffer,
        Texture     mat,
        Texture     norm,
        const Textured::VSUniform& vsuni,
        const Textured::FSUniform& fsuni)
    {
        
    }
    void DrawFlat(
        Buffer buffer,
        const Flat::VSUniform& vsuni,
        const Flat::FSUniform& fsuni)
    {
        
    }
    Buffer CreateBuffer(const BufferDesc& desc)
    {
        
    }
    void DestroyBuffer(Buffer buffer)
    {
        
    }
    Texture CreateTexture(const TextureDesc& desc)
    {
        
    }
    void DestroyTexture(Texture texture)
    {
        
    }
};
