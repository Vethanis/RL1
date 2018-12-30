#include "sky.h"
#include "../shader.h"
#include "../sokol_gfx.h"

namespace Sky
{

const char* sky_vs = 
R"(
#version 330 core

in vec2 position;
out vec2 uv;

void main()
{
    gl_Position = vec4(position, 1.0, 1.0);
    uv = position * 0.5 + 0.5;
}
)";

const char* sky_fs = 
R"(
#version 330 core

in vec2 uv;
out vec4 frag_color;

uniform mat4    IVP;
uniform vec3    Eye;
uniform vec3    LightDir;
uniform float   LightRad;

vec3 ToWorld(float x, float y, float z)
{
    vec4 t = vec4(x, y, z, 1.0);
    t = IVP * t;
    return vec3(t/t.w);
}

vec2 rsi(vec3 r0, vec3 rd, float sr) 
{
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return vec2(1e5,-1e5);
    return vec2(
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    );
}

// https://github.com/wwwtyro/glsl-atmosphere
vec3 atmosphere(
    vec3    L,
    vec3    r, 
    float   iSun, 
    float   rPlanet, 
    float   rAtmos, 
    vec3    kRlh, 
    float   kMie, 
    float   shRlh, 
    float   shMie, 
    float   g)
{
    const int iSteps = 16;
    const int jSteps = 16;
    const float PI = 3.141592;

    vec3 pSun = L;
    const vec3 r0 = vec3(0.0,6372e3,0.0);

    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) 
        return vec3(0.0);
    
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);

    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    vec3 totalRlh = vec3(0.0);
    vec3 totalMie = vec3(0.0);
    float iOdRlh = 0.0;
    float iOdMie = 0.0;
    float iTime = 0.0;
    for (int i = 0; i < iSteps; i++) 
    {
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);
        float iHeight = length(iPos) - rPlanet;
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);
        float jTime = 0.0;
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        for (int j = 0; j < jSteps; j++) 
        {
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);
            float jHeight = length(jPos) - rPlanet;

            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            jTime += jStepSize;
        }

        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        iTime += iStepSize;
    }

    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}

vec3 skylight(vec3 rd, vec3 L, float Rad)
{
    return atmosphere(L, rd, Rad, 6371e3, 6471e3, 
        vec3(5.5e-6, 13.0e-6, 22.4e-6), 
        21e-6, 8e3, 1.2e3, 0.758);
}

vec3 ToneMap(vec3 x)
{
    x = x / (vec3(1.0) + x);
    x = pow(x, vec3(1.0 / 2.2));
    return x;
}

void main()
{
    vec2 UV = uv * 2.0 - 1.0;
    vec3 rd = normalize(ToWorld(UV.x, UV.y, 0.0) - Eye);
    vec3 L = LightDir;
    float Rad = LightRad;
    vec3 C = skylight(rd, L, Rad);
    C = ToneMap(C);
    frag_color = vec4(C, 1.0);
}
)";

sg_shader_desc      shadesc = {0};
sg_pipeline_desc    pdesc = {0};

const void* GetShaderDesc()
{
    int32_t u = 0;
    shadesc.vs.source = sky_vs;
    u = 0;
    shadesc.fs.source = sky_fs;
    shadesc.fs.uniform_blocks[0].size = sizeof(FSUniform);
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "IVP",             SG_UNIFORMTYPE_MAT4   };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Eye",             SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "LightDir",        SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "LightRad",        SG_UNIFORMTYPE_FLOAT  };

    return &shadesc;
}

const void* GetPipelineDesc()
{
    pdesc.shader = Shaders::Get(ST_Sky);
    pdesc.layout.attrs[0].name = "position";
    pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    pdesc.rasterizer.face_winding = SG_FACEWINDING_CCW;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pdesc.depth_stencil.depth_write_enabled = true;

    return &pdesc;
}

};
