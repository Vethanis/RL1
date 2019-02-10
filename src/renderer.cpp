#include "renderer.h"

#include "macro.h"

#if VK_BACKEND
    #include "vkrenderer.h"
#else
    #include "glrenderer.h"
#endif // VK_BACKEND

namespace Renderer
{
    void Init()
    {
        VK_ONLY(VkRenderer::Init());
        GL_ONLY(GLRenderer::Init());
    }
    void Shutdown()
    {
        VK_ONLY(VkRenderer::Shutdown());
        GL_ONLY(GLRenderer::Shutdown());
    }
    void Begin()
    {
        VK_ONLY(VkRenderer::Begin());
        GL_ONLY(GLRenderer::Begin());
    }
    void End()
    {
        VK_ONLY(VkRenderer::End());
        GL_ONLY(GLRenderer::End());
    }
    void SetViewport(const vec4& viewport)
    {
        VK_ONLY(VkRenderer::SetViewport(viewport));
        GL_ONLY(GLRenderer::SetViewport(viewport));
    }
    void DrawBackground()
    {
        VK_ONLY(VkRenderer::DrawBackground());
        GL_ONLY(GLRenderer::DrawBackground());
    }
    void DrawTextured(
        Buffer      verts,
        Buffer      indices,
        Texture     mat,
        Texture     norm,
        const Textured::VSUniform& vsuni,
        const Textured::FSUniform& fsuni)
    {
        VK_ONLY(VkRenderer::DrawTextured(verts, indices, mat, norm, vsuni, fsuni));
        GL_ONLY(GLRenderer::DrawTextured(verts, indices, mat, norm, vsuni, fsuni));
    }
    void DrawFlat(
        Buffer verts,
        Buffer indices,
        const Flat::VSUniform& vsuni,
        const Flat::FSUniform& fsuni)
    {
        VK_ONLY(VkRenderer::DrawFlat(verts, indices, vsuni, fsuni));
        GL_ONLY(GLRenderer::DrawFlat(verts, indices, vsuni, fsuni));
    }
    Buffer CreateBuffer(const BufferDesc& desc)
    {
        VK_ONLY(return VkRenderer::CreateBuffer(desc));
        GL_ONLY(return GLRenderer::CreateBuffer(desc));
    }
    void DestroyBuffer(Buffer buffer)
    {
        VK_ONLY(VkRenderer::DestroyBuffer(buffer));
        GL_ONLY(GLRenderer::DestroyBuffer(buffer));
    }
    Texture CreateTexture(const TextureDesc& desc)
    {
        VK_ONLY(return VkRenderer::CreateTexture(desc));
        GL_ONLY(return GLRenderer::CreateTexture(desc));
    }
    void DestroyTexture(Texture texture)
    {
        VK_ONLY(VkRenderer::DestroyTexture(texture));
        GL_ONLY(GLRenderer::DestroyTexture(texture));
    }
};
