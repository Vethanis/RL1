#include "textured.h"

#include "../sokol_gfx.h"
#include "../shader.h"

namespace Textured
{

const char* textured_vs = 
R"(

#version 330 core

in vec3 position;
in vec3 normal;

out vec3 Position;
out vec3 MacroNormal;

uniform mat4 MVP;
uniform mat4 M;

void main()
{
    mat3 IM     = inverse(mat3(M));
    
    gl_Position = MVP * vec4(position.xyz, 1.0);
    MacroNormal = normalize(normal * IM);
    Position    = vec3(M * vec4(position.xyz, 1.0));
}

)";

const char* textured_fs = 
R"(

#version 330 core

in vec3 Position;
in vec3 MacroNormal;

out vec4 frag_color;

uniform sampler2D MatTex;
uniform sampler2D NorTex;

uniform vec3    Eye;
uniform vec3    LightDir;
uniform vec3    Pal0;
uniform vec3    Pal1;
uniform vec3    Pal2;
uniform float   PalCenter;
uniform float   LightRad;
uniform float   RoughnessOffset;
uniform float   MetalnessOffset;
uniform float   Seed;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

mat3 GetBasis(vec3 V, vec3 N)
{
    vec3 T;
    vec3 B;
    T = normalize(cross(N, -V));
    B = normalize(cross(N, T));
    return mat3(T, B, N);
}

float DisGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdH = max(dot(N, H), 0.0);
    float NdH2 = NdH * NdH;

    float nom = a2;
    float denom_term = (NdH2 * (a2 - 1.0) + 1.0);
    float denom = 3.141592 * denom_term * denom_term;

    return nom / denom;
}

float GeomSchlickGGX(float NdV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdV;
    float denom = NdV * (1.0 - k) + k;

    return nom / denom;
}

float GeomSmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdV = max(dot(N, V), 0.0);
    float NdL = max(dot(N, L), 0.0);
    float ggx2 = GeomSchlickGGX(NdV, roughness);
    float ggx1 = GeomSchlickGGX(NdL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ------------------------------------------------------------------------

vec3 PBRLighting(
    vec3    V, 
    vec3    L, 
    vec3    radiance,
    vec3    N, 
    vec3    albedo, 
    float   metalness, 
    float   roughness)
{
    float NdL = max(0.0, dot(N, L));
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 H = normalize(V + L);

    float NDF = DisGGX(N, H, roughness);
    float G = GeomSmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 nom = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * NdL + 0.001;
    vec3 specular = nom / denom;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metalness);

    return (kD * albedo / 3.141592 + specular) * radiance * NdL;
}

vec3 PaletteToAlbedo(float palette)
{
    if(palette < PalCenter)
    {
        float alpha = palette / PalCenter;
        return mix(Pal0, Pal1, alpha);
    }
    float alpha = (palette - PalCenter) / (1.0 - PalCenter);
    return mix(Pal1, Pal2, alpha);
}

vec3 ToneMap(vec3 x)
{
    x = x / (vec3(1.0) + x);
    x = pow(x, vec3(1.0 / 2.2));
    return x;
}

vec4 MatTriplane(vec3 blending, vec3 P)
{
    vec4 x = texture2D(MatTex, P.yz);
    vec4 y = texture2D(MatTex, P.xz);
    vec4 z = texture2D(MatTex, P.xy);
    return x * blending.x + y * blending.y + z * blending.z;
}

vec4 NorTriplane(vec3 blending, vec3 P)
{
    vec4 x = texture2D(NorTex, P.yz);
    vec4 y = texture2D(NorTex, P.xz);
    vec4 z = texture2D(NorTex, P.xy);
    return x * blending.x + y * blending.y + z * blending.z;
}

vec3 TriplaneBlending(vec3 N)
{
    vec3 blending = abs(N);
    blending = normalize(max(blending, 0.00001));
    float b = (blending.x + blending.y + blending.z);
    blending /= vec3(b, b, b);
    return blending;
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

        for (int j = 0; j < jSteps; j++) {
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

vec3 skylight(vec3 V, vec3 L, float Rad)
{
    return atmosphere(L, -V, Rad, 6371e3, 6471e3, 
        vec3(5.5e-6, 13.0e-6, 22.4e-6), 
        21e-6, 8e3, 1.2e3, 0.758);
}

void main()
{
    vec3 P      = Position;
    vec3 V      = normalize(Eye - P);
    vec3 N      = normalize(MacroNormal);
    vec3 blending = TriplaneBlending(N);

    vec4  PRMA      = MatTriplane(blending, P);
    float palette   = PRMA.x;
    float roughness = clamp(PRMA.y + RoughnessOffset, 0.0, 1.0);
    float metalness = clamp(PRMA.z + MetalnessOffset, 0.0, 1.0);
    vec3  albedo    = PaletteToAlbedo(palette);
    vec3  Ntex      = normalize(NorTriplane(blending, P).xyz * 2.0 - 1.0);
    N               = GetBasis(V, N) * Ntex;

    vec3 C = vec3(0.0);
    {
        // to sun
        vec3 ray = LightDir;
        C += PBRLighting(
            V,
            ray,
            skylight(ray, LightDir, LightRad),
            N,
            albedo,
            metalness,
            roughness
        );
        // to reflection
        ray = reflect(-V, N);
        C += PBRLighting(
            V,
            ray,
            skylight(ray, LightDir, LightRad),
            N,
            albedo,
            metalness,
            roughness
        );
    }

    C += albedo * 0.1;
    C = ToneMap(C);

    //C = 0.5 * N + 0.5;
    //C = N;
    //C = 0.5 * reflect(-V, N) + 0.5;

    frag_color = vec4(C.xyz, 1.0);
}

)";

sg_shader_desc      shadesc = {0};
sg_pipeline_desc    pdesc = {0};

const void* GetShaderDesc()
{
    int32_t u = 0;
    shadesc.vs.source = textured_vs;
    shadesc.vs.uniform_blocks[0].size = sizeof(VSUniform);
    shadesc.vs.uniform_blocks[0].uniforms[u++] = { "MVP", SG_UNIFORMTYPE_MAT4 };
    shadesc.vs.uniform_blocks[0].uniforms[u++] = { "M",   SG_UNIFORMTYPE_MAT4 };
    u = 0;
    shadesc.fs.source = textured_fs;
    shadesc.fs.uniform_blocks[0].size = sizeof(FSUniform);
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Eye",             SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "LightDir",        SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Pal0",            SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Pal1",            SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Pal2",            SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "PalCenter",       SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "LightRad",        SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "RoughnessOffset", SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "MetalnessOffset", SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Seed",            SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.images[0].name = "MatTex";
    shadesc.fs.images[0].type = SG_IMAGETYPE_2D;
    shadesc.fs.images[1].name = "NorTex";
    shadesc.fs.images[1].type = SG_IMAGETYPE_2D;

    return &shadesc;
}

const void* GetPipelineDesc()
{
    pdesc.shader = Shaders::Get(ST_Textured);
    pdesc.index_type = SG_INDEXTYPE_UINT32;
    pdesc.layout.attrs[0].name = "position";
    pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[1].name = "normal";
    pdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.depth_stencil.depth_write_enabled = true;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;
    pdesc.rasterizer.face_winding = SG_FACEWINDING_CCW;

    //pdesc.primitive_type = SG_PRIMITIVETYPE_POINTS;
    
    return &pdesc;
}

};