#pragma once

#include "../linmath.h"

struct VSUniform
{
    mat4 MVP;
    mat4 M;
};

static const char* textured_vs = 
R"(

#version 330 core

in vec3 position;
in vec3 normal;
in vec2 uv0;

out vec3 Position;
out vec3 MacroNormal;
out vec2 uv;

uniform mat4 MVP;
uniform mat4 M;

void main()
{
    mat3 IM     = inverse(mat3(M));
    
    gl_Position = MVP * vec4(position.xyz, 1.0);
    MacroNormal = normalize(normal * IM);
    Position    = vec3(M * vec4(position.xyz, 1.0));
    uv          = uv0;
}

)";

struct FSUniform
{
    vec3  Eye;
    vec3  LightDir;
    vec3  LightRad;
    float BumpScale;
    float ParallaxScale;
};

static const char* textured_fs = 
R"(

#version 330 core

in vec3 Position;
in vec3 MacroNormal;
in vec2 uv;

out vec4 frag_color;

uniform sampler2D MatTex;
uniform sampler2D PalTex;

uniform vec3    Eye;
uniform vec3    LightDir;
uniform vec3    LightRad;
uniform float   BumpScale;
uniform float   ParallaxScale;

float GetHeight(vec2 uv)
{
    return texture(MatTex, uv).y;
}

mat3 GetBasis(vec3 N)
{
    mat3 TBN;
    if(abs(N.x) > 0.001)
        TBN[0] = cross(vec3(0.0, 1.0, 0.0), N);
    else
        TBN[0] = cross(vec3(1.0, 0.0, 0.0), N);
    TBN[0] = normalize(TBN[0]);
    TBN[1] = cross(N, TBN[0]);
    TBN[2] = N;
    return TBN;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir, float scale)
{
    const float numLayers = 10.0;
    const float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy / viewDir.z * scale; 
    vec2 deltaTexCoords = P / numLayers;

    vec2  currentTexCoords = texCoords;
    float currentDepthMapValue = GetHeight(currentTexCoords);
    
    for(float i = 0; i < numLayers && currentLayerDepth < currentDepthMapValue; ++i)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = GetHeight(currentTexCoords);
        currentLayerDepth += layerDepth;  
    }
    
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = GetHeight(prevTexCoords) - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

vec3 Height2N(vec3 P, float h, float scale)
{
    float dhdx = dFdx(h);
    float dhdy = dFdy(h);

    vec3 dpdx = dFdx(P);
    vec3 dpdy = dFdy(P);

    vec3 r1 = cross(dpdy, MacroNormal);
    vec3 r2 = cross(MacroNormal, dpdx);

    vec3 g = (r1 * dhdx + r2 * dhdy) / dot(dpdx, r1);

    return normalize(MacroNormal + -g * scale);
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

vec3 ToneMap(vec3 x)
{
    x = x / (vec3(1.0) + x);
    x = pow(x, vec3(1.0 / 2.2));
    return x;
}

void main()
{
    vec3 P      = Position;
    vec3 V      = normalize(Eye - P);
    vec3 tanV   = transpose(GetBasis(MacroNormal)) * V;
    vec2 UV     = ParallaxMapping(uv, tanV, ParallaxScale);

    vec4 PHRM       = texture(MatTex, UV);
    float palette   = PHRM.x;
    float height    = PHRM.y;
    float roughness = PHRM.z;
    float metalness = PHRM.w;
    vec3 albedo     = texture(PalTex, vec2(palette, 0.0)).xyz;
    vec3 N          = Height2N(P, height, BumpScale);

    vec3 C          = PBRLighting(
        V,
        LightDir,
        LightRad,
        N,
        albedo,
        metalness,
        roughness
    );

    C += albedo * 0.1;
    C = ToneMap(C);

    //C = N;

    frag_color = vec4(C.xyz, 1.0);
}

)";
