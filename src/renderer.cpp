#include "renderer.h"

#include "macro.h"
#include "ecs.h"
#include "component_types.h"

def u32 NumTextures = 10;
def u32 TextureSize = 64;
def u32 TargetSize = 256;

struct Texture
{
    u8 values[TextureSize * TextureSize * 4];
};

struct Target
{
    f32 color[TargetSize * TargetSize * 4];
    f32 depth[TargetSize * TargetSize];
};

struct Resolve
{
    u8 values[TargetSize * TargetSize * 4];
};

static Target   ms_targets[2];
static Resolve  ms_resolve;
static Texture  ms_textures[NumTextures];
static u32      ms_frame = 0;

namespace Renderer
{
    void Init()
    {

    }
    void Update()
    {
        let positions = ToConst(ECS::GetAll<Position>());
        let scales = ToConst(ECS::GetAll<Scale>());
        let orientations = ToConst(ECS::GetAll<Orientation>());
        let infos = ECS::GetAll<DrawInfo>();

        let flags = ECS::GetFlags();
        for(usize i = 0; i < flags.size(); ++i)
        {
            if(!flags[i].has(CT_Position))
            {
                continue;
            }
            if(!flags[i].has(CT_DrawInfo))
            {
                continue;
            }


        }
    }
    void Shutdown()
    {

    }
};
