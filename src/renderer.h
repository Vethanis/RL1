#pragma once

#include "slot.h"
#include "slice.h"
#include "hlsl_types.h"

namespace Renderer
{
    void Init();
    void Shutdown();

    void Begin();
    void Draw();
    void End();
};

enum class BufferType
{
    Vertices = 0,
    Indices,
    Bytes,

    Count
};

enum class TextureFormat
{
    RGBA8 = 0,
    RGBA16F,
    RGBA32F,
    R32F,
    RG32F,
    RGB32F,
    D32,

    Count
};

enum class TextureType
{
    D1 = 0,
    D2,
    D3,
    Cube,
    Depth,

    Count
};

enum class SamplerFilter
{
    Nearest = 0,
    Bilinear,
    Trilinear,
    Anisotropic_2X,
    Anisotropic_4X,

    Count
};

enum class SamplerWrap
{
    Clamp = 0,
    Repeat,
    MirroredRepeat,

    Count
};

struct Buffer { slot id; uint32_t rc; };
struct BufferDesc
{
    void*           elements;
    uint32_t        elementSize;
    uint32_t        elementCount;
    BufferType      type;
};

struct SamplerDesc
{
    SamplerFilter minFilter;
    SamplerFilter magFilter;
    SamplerWrap   wrap;
};

struct Texture { slot id; uint32_t rc; };
struct TextureDesc
{
    void*           data;
    uint32_t        width; 
    uint32_t        height;
    uint32_t        layers;
    TextureType     type;
    TextureFormat   format;
    SamplerDesc     sampler;
};

struct Pipeline { slot id; uint32_t rc; };
struct PipelineDesc
{
    Slice<uint32_t> vertexSrc;
    Slice<uint32_t> fragmentSrc;
};

struct Uniforms
{
    float4 data[12];
};

Pipeline CreatePipeline(const PipelineDesc& desc);
Buffer   CreateBuffer(const BufferDesc& desc);
Texture  CreateTexture(const TextureDesc& desc);

void DestroyPipelines(Slice<Pipeline> pipelines);
void DestroyBuffers(Slice<Buffer> buffers);
void DestroyTextures(Slice<Texture> textures);

struct RenderComponentDesc
{

};

struct RenderComponent
{
    Uniforms    unis;
    Pipeline    pipe;
    Buffer      verts;
    Buffer      indices;
    Texture     albedo;
    Texture     normal;
    Texture     material;
};

void New(RenderComponent& rc, const RenderComponentDesc* desc);
void Drop(RenderComponent& rc);
