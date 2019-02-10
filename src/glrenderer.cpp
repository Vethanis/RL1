#include "glrenderer.h"

#include "linmath.h"
#include "macro.h"
#include "shader.h"
#include "window.h"
#include "vertex.h"
#include "ui.h"
#include "camera.h"
#include "shaders/ibl.h"

#include "gen_array.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"
#include "stb_image.h"

namespace GLRenderer
{

struct GLBuffer
{
    uint32_t id;
    uint32_t count;
};

struct GLTexture
{
    uint32_t id;
};

struct EnvironmentMap
{
    uint32_t m_environmentMap;
    uint32_t m_prefilterMap;
    uint32_t m_irradianceMap;
};

enum
{
    NumFaces = 6,
};

static const mat4 ms_captureViews[NumFaces] = 
{
    glm::lookAt(vec3(0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(vec3(0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(vec3(0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(vec3(0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(vec3(0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(vec3(0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f)),
};
static const mat4           ms_captureProjection = glm::perspective(
    glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
static uint32_t             ms_captureFBO = 0;
static uint32_t             ms_captureRBO = 0;
static uint32_t             ms_brdfLUT = 0;
static uint32_t             ms_cubeVBO = 0;
static uint32_t             ms_quadVBO = 0;
static uint32_t             ms_defaultVAO = 0;
static int32_t              ms_winWidth = 0;
static int32_t              ms_winHeight = 0;
static GLShader             ms_flatShader;
static GLShader             ms_texturedShader;
static GLShader             ms_rect2CMShader;
static GLShader             ms_irradianceShader;
static GLShader             ms_prefilterShader;
static GLShader             ms_brdfShader;
static GLShader             ms_backgroundShader;
static EnvironmentMap       ms_envmap;
static gen_array<GLBuffer>  ms_buffers;
static gen_array<GLTexture> ms_textures;

void InitCube();
void InitQuad();
void RenderCube();
void RenderQuad();
EnvironmentMap CreateEnvironmentMap(const char* hdrmap);
void DestroyEnvironmentMap(EnvironmentMap& map);

void CubeAttrib()
{
    glEnableVertexAttribArray(0); DebugGL();
    glEnableVertexAttribArray(1); DebugGL();
    glEnableVertexAttribArray(2); DebugGL();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); DebugGL();
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); DebugGL();
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); DebugGL();
}

void QuadAttrib()
{
    glEnableVertexAttribArray(0); DebugGL();
    glEnableVertexAttribArray(1); DebugGL();
    glDisableVertexAttribArray(2); DebugGL();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); DebugGL();
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); DebugGL();
}

void VertexAttrib()
{
    glEnableVertexAttribArray(0); DebugGL();
    glEnableVertexAttribArray(1); DebugGL();
    glDisableVertexAttribArray(2); DebugGL();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); DebugGL();
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3)); DebugGL();
}

void Init()
{
    // setup opengl state
    glEnable(GL_MULTISAMPLE); DebugGL();
    glEnable(GL_DEPTH_TEST); DebugGL();
    glDepthFunc(GL_LEQUAL); DebugGL();
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); DebugGL();

    glCreateVertexArrays(1, &ms_defaultVAO); DebugGL();
    glBindVertexArray(ms_defaultVAO); DebugGL();
    InitCube();
    InitQuad();

    // setup shaders
    ms_flatShader.Init(flat_vs, flat_fs);
    ms_texturedShader.Init(textured_vs, textured_fs);
    ms_rect2CMShader.Init(cubemap_vs, rect2CM_fs);
    ms_irradianceShader.Init(cubemap_vs, irradiance_convolution_fs);
    ms_prefilterShader.Init(cubemap_vs, prefilter_fs);
    ms_brdfShader.Init(brdf_vs, brdf_fs);
    ms_backgroundShader.Init(background_vs, background_fs);

    ms_flatShader.Use();
    ms_flatShader.SetInt("irradianceMap", 0);
    ms_flatShader.SetInt("prefilterMap", 1);
    ms_flatShader.SetInt("brdfLUT", 2);

    ms_texturedShader.Use();
    ms_texturedShader.SetInt("irradianceMap", 0);
    ms_texturedShader.SetInt("prefilterMap", 1);
    ms_texturedShader.SetInt("brdfLUT", 2);
    ms_texturedShader.SetInt("MatTex", 3);
    ms_texturedShader.SetInt("NorTex", 4);

    ms_backgroundShader.Use();
    ms_backgroundShader.SetInt("environmentMap", 0);

    // setup framebuffer; these are persistent
    glGenFramebuffers(1, &ms_captureFBO); DebugGL();
    glGenRenderbuffers(1, &ms_captureRBO);     DebugGL();

    {
        const uint32_t  LUTScale = 512;
        // generate a 2d LUT from the brdf equations used
        glGenTextures(1, &ms_brdfLUT); DebugGL();
        glBindTexture(GL_TEXTURE_2D, ms_brdfLUT); DebugGL();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, LUTScale, LUTScale, 0, GL_RG, GL_FLOAT, 0); DebugGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); DebugGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); DebugGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); DebugGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); DebugGL();

        glBindFramebuffer(GL_FRAMEBUFFER, ms_captureFBO); DebugGL();
        glBindRenderbuffer(GL_RENDERBUFFER, ms_captureRBO); DebugGL();
        // creates depth target that we cant sample later that fits LUT size
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, LUTScale, LUTScale); DebugGL();
        // sets texture as color target that we can sample later
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_2D, ms_brdfLUT, 0);     DebugGL();
            
        glViewport(0, 0, LUTScale, LUTScale); DebugGL();
        ms_brdfShader.Use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); DebugGL();
        RenderQuad(); DebugGL();
        glBindFramebuffer(GL_FRAMEBUFFER, 0); DebugGL();
    }

    ms_envmap = CreateEnvironmentMap("Factory_Catwalk_2k");
}

EnvironmentMap CreateEnvironmentMap(const char* hdrmap)
{
    const uint32_t  irradianceScale = 32;
    const uint32_t  prefilterScale  = 128;
    const uint32_t  cubemapScale    = 512;
    const uint32_t  maxMipLevels    = 5;

    EnvironmentMap map;
    MemZero(map);
    // load hdr env map
    char path[MAX_PATH_LEN] = {0};
    Format(path, "assets/images/%s.hdr", hdrmap);
    stbi_set_flip_vertically_on_load(true);
    int32_t txWidth, txHeight, txComps;
    float* data = stbi_loadf(path, &txWidth, &txHeight, &txComps, 0);
    Assert(data);

    uint32_t hdrTexture = 0;
    glGenTextures(1, &hdrTexture); DebugGL();
    glBindTexture(GL_TEXTURE_2D, hdrTexture); DebugGL();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, txWidth, txHeight, 0, GL_RGB, GL_FLOAT, data); DebugGL();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); DebugGL();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); DebugGL();
    stbi_image_free(data);

    // setup cubemap to render to and attach to framebuffer
    glGenTextures(1, &map.m_environmentMap); DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, map.m_environmentMap); DebugGL();
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
            GL_RGB16F, cubemapScale, cubemapScale, 0, GL_RGB, GL_FLOAT, nullptr); DebugGL();
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); DebugGL();

    // convert hdr env map to cubemap
    ms_rect2CMShader.Use();
    ms_rect2CMShader.SetInt("equirectangularMap", 0);
    ms_rect2CMShader.SetMat4("projection", ms_captureProjection);
    glActiveTexture(GL_TEXTURE0); DebugGL();
    glBindTexture(GL_TEXTURE_2D, hdrTexture); DebugGL();
    
    glBindFramebuffer(GL_FRAMEBUFFER, ms_captureFBO); DebugGL();
    glBindRenderbuffer(GL_RENDERBUFFER, ms_captureRBO); DebugGL();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubemapScale, cubemapScale); DebugGL();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ms_captureRBO); DebugGL();

    glViewport(0, 0, cubemapScale, cubemapScale); DebugGL();
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        ms_rect2CMShader.SetMat4("view", ms_captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, map.m_environmentMap, 0); DebugGL();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); DebugGL();

        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); DebugGL();

    glBindTexture(GL_TEXTURE_CUBE_MAP, map.m_environmentMap); DebugGL();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP); DebugGL();

    glDeleteTextures(1, &hdrTexture); DebugGL();

    // create irradiance cubemap
    glGenTextures(1, &map.m_irradianceMap); DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, map.m_irradianceMap); DebugGL();
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
            GL_RGB16F, irradianceScale, irradianceScale, 0, GL_RGB, GL_FLOAT, nullptr); DebugGL();
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); DebugGL();

    glBindFramebuffer(GL_FRAMEBUFFER, ms_captureFBO); DebugGL();
    glBindRenderbuffer(GL_RENDERBUFFER, ms_captureRBO); DebugGL();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceScale, irradianceScale); DebugGL();

    // solve diffuse integral for irradiance cubemap
    ms_irradianceShader.Use();
    ms_irradianceShader.SetInt("environmentMap", 0);
    ms_irradianceShader.SetMat4("projection", ms_captureProjection);
    glActiveTexture(GL_TEXTURE0); DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, map.m_environmentMap); DebugGL();

    glViewport(0, 0, irradianceScale, irradianceScale); DebugGL();
    glBindFramebuffer(GL_FRAMEBUFFER, ms_captureFBO); DebugGL();
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        ms_irradianceShader.SetMat4("view", ms_captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, map.m_irradianceMap, 0); DebugGL();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); DebugGL();

        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); DebugGL();

    // create a prefilter cubemap, and rescale capture FBO to prefilter scale
    glGenTextures(1, &map.m_prefilterMap); DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, map.m_prefilterMap); DebugGL();
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 
            prefilterScale, prefilterScale, 0, GL_RGB, GL_FLOAT, nullptr); DebugGL();
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  DebugGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); DebugGL();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP); DebugGL();

    // run a quasi monte-carlo sim on the environment lighting to create a prefilter cubemap
    ms_prefilterShader.Use();
    ms_prefilterShader.SetInt("environmentMap", 0);
    ms_prefilterShader.SetMat4("projection", ms_captureProjection);
    glActiveTexture(GL_TEXTURE0); DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, map.m_environmentMap); DebugGL();

    glBindFramebuffer(GL_FRAMEBUFFER, ms_captureFBO); DebugGL();
    glBindRenderbuffer(GL_RENDERBUFFER, ms_captureRBO); DebugGL();
    for(uint32_t mip = 0; mip < maxMipLevels; ++mip)
    {
        uint32_t mipWidth = prefilterScale >> mip;
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipWidth); DebugGL();
        glViewport(0, 0, mipWidth, mipWidth); DebugGL();

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        ms_prefilterShader.SetFloat("roughness", roughness);
        for(uint32_t i = 0; i < NumFaces; ++i)
        {
            ms_prefilterShader.SetMat4("view", ms_captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, map.m_prefilterMap, mip); DebugGL();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); DebugGL();
            
            RenderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); DebugGL();

    return map;
}

void DestroyEnvironmentMap(EnvironmentMap& map)
{
    glDeleteTextures(1, &map.m_environmentMap); DebugGL();
    glDeleteTextures(1, &map.m_irradianceMap); DebugGL();
    glDeleteTextures(1, &map.m_prefilterMap); DebugGL();
    MemZero(map);
}

void InitCube()
{
    const float vertices[] = 
    {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
    };
    glGenBuffers(1, &ms_cubeVBO); DebugGL();
    glBindBuffer(GL_ARRAY_BUFFER, ms_cubeVBO); DebugGL();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); DebugGL();
}

void InitQuad()
{
    const float quadVertices[] = 
    {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenBuffers(1, &ms_quadVBO); DebugGL();
    glBindBuffer(GL_ARRAY_BUFFER, ms_quadVBO); DebugGL();
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW); DebugGL();
}

void RenderCube()
{
    glBindBuffer(GL_ARRAY_BUFFER, ms_cubeVBO); DebugGL();
    CubeAttrib();
    glDrawArrays(GL_TRIANGLES, 0, 36); DebugGL();
}

void RenderQuad()
{
    glBindBuffer(GL_ARRAY_BUFFER, ms_quadVBO); DebugGL();
    QuadAttrib();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); DebugGL();
}

void Begin()
{
    Window::GetSize(Window::GetActive(), ms_winWidth, ms_winHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); DebugGL();
    SetViewport(vec4(0.0f, 0.0f, 1.0f, 1.0f));
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f); DebugGL();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); DebugGL();
    glEnable(GL_CULL_FACE); DebugGL();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); DebugGL();
    glActiveTexture(GL_TEXTURE0 + 0); DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, ms_envmap.m_irradianceMap); DebugGL();
    glActiveTexture(GL_TEXTURE0 + 1); DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, ms_envmap.m_prefilterMap); DebugGL();
    glActiveTexture(GL_TEXTURE0 + 2); DebugGL();
    glBindTexture(GL_TEXTURE_2D, ms_brdfLUT); DebugGL();
}

void DrawBackground()
{
    const Camera* cam = Camera::GetActive();
    ms_backgroundShader.Use();
    ms_backgroundShader.SetMat4("projection", cam->P);
    ms_backgroundShader.SetMat4("view", cam->V);
    glActiveTexture(GL_TEXTURE0 + 0); DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, ms_envmap.m_environmentMap); DebugGL();
    //glBindTexture(GL_TEXTURE_CUBE_MAP, ms_envmap.m_irradianceMap); // display irradiance map
    //glBindTexture(GL_TEXTURE_CUBE_MAP, ms_envmap.m_prefilterMap); // display prefilter map
    glDisable(GL_CULL_FACE); DebugGL();
    RenderCube();
    glEnable(GL_CULL_FACE); DebugGL();
}

void End()
{
    UI::End();
    Window::Swap(Window::GetActive());
}

void SetViewport(const vec4& viewport)
{
    ivec4 px = ivec4(vec4(ms_winWidth, ms_winHeight, ms_winWidth, ms_winHeight) * viewport);
    glViewport(px[0], px[1], px[2], px[3]); DebugGL();
}

inline void Draw(GLBuffer verts)
{
    glBindBuffer(GL_ARRAY_BUFFER, verts.id); DebugGL();
    VertexAttrib();
    glDrawArrays(GL_TRIANGLES, 0, verts.count); DebugGL();
}

inline void Draw(GLBuffer verts, GLBuffer inds)
{
    glBindBuffer(GL_ARRAY_BUFFER, verts.id); DebugGL();
    VertexAttrib();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, inds.id); DebugGL();
    glDrawElements(GL_TRIANGLES, inds.count, GL_UNSIGNED_INT, 0); DebugGL();
}

inline void Draw(Renderer::Buffer verts, Renderer::Buffer inds)
{
    if(const GLBuffer* x = ms_buffers.Get(verts.id))
    {
        if(const GLBuffer* y = ms_buffers.Get(inds.id))
        {
            Draw(*x, *y);
        }
        else
        {
            Draw(*x);
        }
    }
}

inline void Bind2D(GLTexture tex, int32_t slot)
{
    glActiveTexture(GL_TEXTURE0 + slot); DebugGL();
    glBindTexture(GL_TEXTURE_2D, tex.id); DebugGL();
}

inline void Bind2D(Renderer::Texture tex, int32_t slot)
{
    if(const GLTexture* x = ms_textures.Get(tex.id))
    {
        Bind2D(*x, slot);
    }
}

void DrawTextured(
    Renderer::Buffer            verts,
    Renderer::Buffer            inds,
    Renderer::Texture           mat,
    Renderer::Texture           norm,
    const Textured::VSUniform&  vsuni,
    const Textured::FSUniform&  fsuni)
{
    ms_texturedShader.Use();
    ms_texturedShader.SetMat4("MVP", vsuni.MVP);
    ms_texturedShader.SetMat4("M", vsuni.M);
    ms_texturedShader.SetVec3("Eye", fsuni.Eye);
    ms_texturedShader.SetVec3("LightDir", fsuni.LightDir);
    ms_texturedShader.SetFloat("LightRad", fsuni.LightRad);
    ms_texturedShader.SetVec3("Pal0", fsuni.Pal0);
    ms_texturedShader.SetVec3("Pal1", fsuni.Pal1);
    ms_texturedShader.SetVec3("Pal2", fsuni.Pal2);
    ms_texturedShader.SetFloat("PalCenter", fsuni.PalCenter);
    ms_texturedShader.SetFloat("RoughnessOffset", fsuni.RoughnessOffset);
    ms_texturedShader.SetFloat("MetalnessOffset", fsuni.MetalnessOffset);
    ms_texturedShader.SetFloat("Seed", fsuni.Seed);

    Bind2D(mat, 3);
    Bind2D(norm, 4);
    Draw(verts, inds);
}

void DrawFlat(
    Renderer::Buffer       verts,
    Renderer::Buffer       inds,
    const Flat::VSUniform& vsuni,
    const Flat::FSUniform& fsuni)
{
    ms_flatShader.Use();
    ms_flatShader.SetMat4("MVP", vsuni.MVP);
    ms_flatShader.SetMat4("M", vsuni.M);
    ms_flatShader.SetVec3("Eye", fsuni.Eye);
    ms_flatShader.SetVec3("LightDir", fsuni.LightDir);
    ms_flatShader.SetVec3("Albedo", fsuni.Albedo);
    ms_flatShader.SetFloat("LightRad", fsuni.LightRad);
    ms_flatShader.SetFloat("Roughness", fsuni.Roughness);
    ms_flatShader.SetFloat("Metalness", fsuni.Metalness);
    ms_flatShader.SetFloat("Seed", fsuni.Seed);

    Draw(verts, inds);
}

Renderer::Buffer CreateBuffer(const Renderer::BufferDesc& desc)
{
    GLBuffer buffer;
    buffer.count = desc.count;

    switch(desc.type)
    {
        case Renderer::BufferType::Vertices:
        Assert(desc.data);
        glGenBuffers(1, &buffer.id); DebugGL();
        glBindBuffer(GL_ARRAY_BUFFER, buffer.id); DebugGL();
        glBufferData(GL_ARRAY_BUFFER, desc.stride * desc.count, desc.data, GL_STATIC_DRAW); DebugGL();
        break;
        case Renderer::BufferType::Indices:
        Assert(desc.data);
        glGenBuffers(1, &buffer.id); DebugGL();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id); DebugGL();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc.stride * desc.count, desc.data, GL_STATIC_DRAW); DebugGL();
        break;
        default:
        Assert(false);
    }

    slot s = ms_buffers.Create();
    ms_buffers.GetUnchecked(s) = buffer;

    return { s };
}

void DestroyBuffer(Renderer::Buffer buffer)
{
    if(GLBuffer* x = ms_buffers.Get(buffer.id))
    {
        glDeleteBuffers(1, &(x->id)); DebugGL();
        ms_buffers.DestroyUnchecked(buffer.id);
    }
}

Renderer::Texture CreateTexture(const Renderer::TextureDesc& desc)
{
    GLTexture texture;
    glGenTextures(1, &texture.id); DebugGL();

    uint32_t glFormat;
    uint32_t glComponents;
    uint32_t glElementType;
    size_t   elementSize;
    switch(desc.format)
    {
        case Renderer::Format::RGBA8:
        glFormat = GL_RGBA8;
        glComponents = GL_RGBA;
        glElementType = GL_UNSIGNED_BYTE;
        elementSize = sizeof(uint8_t) * 4;
        break;
        case Renderer::Format::RGBA16F:
        glFormat = GL_RGBA16F;
        glComponents = GL_RGBA;
        glElementType = GL_FLOAT;
        elementSize = sizeof(uint16_t) * 4;
        break;
        case Renderer::Format::RGBA32F:
        glFormat = GL_RGBA32F;
        glComponents = GL_RGBA;
        glElementType = GL_FLOAT;
        elementSize = sizeof(float) * 4;
        break;
        case Renderer::Format::R32F:
        glFormat = GL_R32F;
        glComponents = GL_RED;
        glElementType = GL_FLOAT;
        elementSize = sizeof(float) * 1;
        break;
        case Renderer::Format::RG32F:
        glFormat = GL_RG32F;
        glComponents = GL_RG;
        glElementType = GL_FLOAT;
        elementSize = sizeof(float) * 2;
        break;
        case Renderer::Format::RGB32F:
        glFormat = GL_RGB32F;
        glComponents = GL_RGB;
        glElementType = GL_FLOAT;
        elementSize = sizeof(float) * 3;
        break;
    }
    uint32_t glWrapType;
    switch(desc.wrapType)
    {
        case Renderer::WrapType::Clamp:
        glWrapType = GL_CLAMP_TO_EDGE;
        break;
        case Renderer::WrapType::Repeat:
        glWrapType = GL_REPEAT;
        break;
    }
    uint32_t glminFilterType;
    switch(desc.minFilter)
    {
        case Renderer::FilterType::Nearest:
        glminFilterType = GL_NEAREST;
        break;
        case Renderer::FilterType::Linear:
        glminFilterType = GL_LINEAR;
        break;
        case Renderer::FilterType::LinearMipmap:
        glminFilterType = GL_LINEAR_MIPMAP_LINEAR;
        break;
    }
    uint32_t glmagFilterType;
    switch(desc.magFilter)
    {
        case Renderer::FilterType::Nearest:
        glmagFilterType = GL_NEAREST;
        break;
        case Renderer::FilterType::Linear:
        glmagFilterType = GL_LINEAR;
        break;
    }

    uint32_t glTarget;
    switch(desc.type)
    {
        case Renderer::TextureType::Texture1D:
        glTarget = GL_TEXTURE_1D;
        glBindTexture(glTarget, texture.id); DebugGL();
        glTexImage1D(glTarget, 0, glFormat, desc.width, 0, glComponents, glElementType, desc.data); DebugGL();
        break;
        case Renderer::TextureType::Texture2D:
        glTarget = GL_TEXTURE_2D;
        glBindTexture(glTarget, texture.id); DebugGL();
        glTexImage2D(glTarget, 0, glFormat, desc.width, desc.height, 0, glComponents, glElementType, desc.data); DebugGL();
        break;
        case Renderer::TextureType::Texture3D:
        glTarget = GL_TEXTURE_3D;
        glBindTexture(glTarget, texture.id); DebugGL();
        glTexImage3D(glTarget, 0, glFormat, desc.width, desc.height, desc.layers, 0, glComponents, glElementType, desc.data); DebugGL();
        break;
        case Renderer::TextureType::TextureCube:
        glTarget = GL_TEXTURE_CUBE_MAP;
        glBindTexture(glTarget, texture.id); DebugGL();
        const uint8_t* pData = (const uint8_t*)desc.data;
        size_t stride = desc.width * desc.height * elementSize;
        for(uint32_t i = 0; i < NumFaces; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glFormat, desc.width, desc.height, 0, glComponents, glElementType, pData + stride * i); DebugGL();
        }
        break;
    }

    const uint32_t wrapDims[] = 
    {
        GL_TEXTURE_WRAP_S,
        GL_TEXTURE_WRAP_T,
        GL_TEXTURE_WRAP_R,
    };

    for(uint32_t dim = 0; dim < 3; ++dim)
    {
        glTexParameteri(glTarget, wrapDims[dim], glWrapType); DebugGL();
    }

    glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, glminFilterType);  DebugGL();
    glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, glmagFilterType); DebugGL();

    if(desc.minFilter == Renderer::FilterType::LinearMipmap)
    {
        glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);  DebugGL();
        glGenerateMipmap(glTarget); DebugGL();
    }

    slot s = ms_textures.Create();
    ms_textures.GetUnchecked() = texture;

    return { s };
}

void DestroyTexture(Renderer::Texture texture)
{
    if(GLTexture* x = ms_textures.Get(texture.id))
    {
        glDeleteTextures(1, &(x->id)); DebugGL();
        ms_textures.DestroyUnchecked(texture.id);
    }
}

void Shutdown()
{

}

};
