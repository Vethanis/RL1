#include "textured.h"

const char* textured_vs = 
R"(

#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 Position;
out vec3 MacroNormal;
out vec3 LocalPosition;

uniform mat4 MVP;
uniform mat4 M;

void main()
{
    mat3 IM     = inverse(mat3(M));
    
    gl_Position = MVP * vec4(position.xyz, 1.0);
    MacroNormal = normalize(normal * IM);
    Position    = vec3(M * vec4(position.xyz, 1.0));
    LocalPosition = position;
}

)";

const char* textured_fs = 
R"(

#version 330 core

in vec3 Position;
in vec3 MacroNormal;
in vec3 LocalPosition;

out vec4 frag_color;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
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

mat3 GetBasis(vec3 P, vec3 N)
{
    vec3 T = normalize(dFdx(LocalPosition));
    vec3 B = normalize(dFdy(LocalPosition));
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

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
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

vec3 TriplaneBlending(vec3 N)
{
    vec3 blending = abs(N);
    blending = normalize(max(blending, 0.00001));
    float b = (blending.x + blending.y + blending.z);
    blending /= vec3(b, b, b);
    return blending;
}

vec4 MatTriplane(vec3 blending, vec3 P)
{
    vec4 x = texture2D(MatTex, P.zy);
    vec4 y = texture2D(MatTex, P.xz);
    vec4 z = texture2D(MatTex, P.xy);
    return x * blending.x + y * blending.y + z * blending.z;
}

vec3 RNMBlend(vec3 n1, vec3 n2)
{
    n1 += vec3(0.0, 0.0, 1.0);
    n2 *= vec3(-1.0, -1.0, 1.0);
    return n1 * dot(n1, n2) / n1.z - n2;
}

vec3 NorTriplane(vec3 blending, vec3 P, vec3 N)
{
    vec3 x = texture2D(NorTex, P.zy).xyz * 2.0 - 1.0;
    vec3 y = texture2D(NorTex, P.xz).xyz * 2.0 - 1.0;
    vec3 z = texture2D(NorTex, P.xy).xyz * 2.0 - 1.0;

    vec3 AVN = abs(N);

    x = RNMBlend(vec3(N.zy, AVN.x), x);
    y = RNMBlend(vec3(N.xz, AVN.y), y);
    z = RNMBlend(vec3(N.xy, AVN.z), z);

    vec3 axisSign = sign(N);

    x.z *= axisSign.x;
    y.z *= axisSign.y;
    z.z *= axisSign.z;

    return normalize(
        x * blending.x + 
        y * blending.y + 
        z * blending.z + 
        N);
}

vec3 Dither(vec3 x)
{
    vec2 s = gl_FragCoord.xy + vec2(Seed);
    const float amt = 1.0 / 255.0;
    for(int i = 0; i < 3; ++i)
    {
        x[i] *= (1.0 - amt);
        x[i] += rand(s) * amt;
        s += vec2(Seed);
    }
    return x;
}

void main()
{
    vec3 P      = Position;
    vec3 V      = normalize(Eye - P);
    vec3 N      = normalize(MacroNormal);
    mat3 TBN    = GetBasis(P, N);
    vec3 blending = TriplaneBlending(N);

    vec4  PRMA      = MatTriplane(blending, LocalPosition);
    float palette   = PRMA.x;
    float roughness = clamp(PRMA.y + RoughnessOffset, 0.05, 1.0);
    float metalness = clamp(PRMA.z + MetalnessOffset, 0.0, 1.0);
    vec3  albedo    = PaletteToAlbedo(palette);
    N               = NorTriplane(blending, LocalPosition, N);

    vec3 C = vec3(0.0);
    {
        // to sun
        C += PBRLighting(
            V,
            LightDir,
            vec3(1.0) * LightRad,
            N,
            albedo,
            metalness,
            roughness
        );
    }
    {
        // ambient IBL
        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metalness);
        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metalness;
        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = irradiance * albedo;
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 R = reflect(-V, N);
        vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
        vec3 ambient = (kD * diffuse + specular);
        C += ambient;
    }

    C = ToneMap(C);
    C = Dither(C);

    //C = 0.5 * N + 0.5;
    //C = N;

    frag_color = vec4(C.xyz, 1.0);
}

)";
