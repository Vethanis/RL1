#include "renderer.h"

#include "linmath.h"
#include "macro.h"
#include "shader.h"
#include "window.h"
#include "vertex.h"
#include "ui.h"
#include "camera.h"
#include "shaders/ibl.h"
#include "shaders/textured.h"

#include "glad.h"
#include <GLFW/glfw3.h>
#include "stb_image.h"

/*
    Heavily derived from https://learnopengl.com/PBR/IBL/Specular-IBL, all credit to them
*/

enum
{
    NumFaces = 6,
};

static const mat4 captureViews[NumFaces] = 
{
    glm::lookAt(vec3(0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(vec3(0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(vec3(0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(vec3(0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(vec3(0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(vec3(0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f)),
};
static const mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
const uint32_t irradianceScale = 32;
const uint32_t prefilterScale = 128;
const uint32_t LUTScale = 512;
const uint32_t cubemapScale = 512;
const uint32_t maxMipLevels = 5;
uint32_t color_cubemap = 0;
uint32_t captureFBO = 0;
uint32_t captureRBO = 0;
uint32_t hdrTexture = 0;
uint32_t envCubemap = 0;
uint32_t irradianceMap = 0;
uint32_t prefilterMap = 0;
uint32_t brdfLUTTexture = 0;
uint32_t DefaultVAO = 0;
GLShader texturedShader;
GLShader rect2CMShader;
GLShader irradianceShader;
GLShader prefilterShader;
GLShader brdfShader;
GLShader backgroundShader;

void RenderCube();
void RenderQuad();

void Renderer::Init()
{
    // setup opengl state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // setup shaders
    texturedShader.Init(textured_vs, textured_fs);
    rect2CMShader.Init(cubemap_vs, rect2CM_fs);
    irradianceShader.Init(cubemap_vs, irradiance_convolution_fs);
    prefilterShader.Init(cubemap_vs, prefilter_fs);
    brdfShader.Init(brdf_vs, brdf_fs);
    backgroundShader.Init(background_vs, background_fs);

    texturedShader.Use();
    texturedShader.SetInt("irradianceMap", 0);
    texturedShader.SetInt("prefilterMap", 1);
    texturedShader.SetInt("brdfLUT", 2);
    texturedShader.SetInt("MatTex", 3);
    texturedShader.SetInt("NorTex", 4);

    backgroundShader.Use();
    backgroundShader.SetInt("environmentMap", 0);

    // setup framebuffer
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubemapScale, cubemapScale);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // load hdr env map
    stbi_set_flip_vertically_on_load(true);
    int32_t txWidth, txHeight, txComps;
    float* data = stbi_loadf(
        "assets/images/Factory_Catwalk_2k.hdr", &txWidth, &txHeight, &txComps, 0);
    Assert(data);

    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, txWidth, txHeight, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);

    // setup cubemap to render to and attach to framebuffer
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
            GL_RGB16F, cubemapScale, cubemapScale, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // convert hdr env map to cubemap
    rect2CMShader.Use();
    rect2CMShader.SetInt("equirectangularMap", 0);
    rect2CMShader.SetMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, cubemapScale, cubemapScale);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        rect2CMShader.SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // create irradiance cubemap
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
            GL_RGB16F, irradianceScale, irradianceScale, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceScale, irradianceScale);

    // solve diffuse integral for irradiance cubemap
    irradianceShader.Use();
    irradianceShader.SetInt("environmentMap", 0);
    irradianceShader.SetMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, irradianceScale, irradianceScale);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        irradianceShader.SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create a prefilter cubemap, and rescale capture FBO to prefilter scale
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for(uint32_t i = 0; i < NumFaces; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 
            prefilterScale, prefilterScale, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // run a quasi monte-carlo sim on the environment lighting to create a prefilter cubemap
    prefilterShader.Use();
    prefilterShader.SetInt("environmentMap", 0);
    prefilterShader.SetMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for(uint32_t mip = 0; mip < maxMipLevels; ++mip)
    {
        uint32_t mipWidth = prefilterScale >> mip;
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipWidth);
        glViewport(0, 0, mipWidth, mipWidth);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.SetFloat("roughness", roughness);
        for(uint32_t i = 0; i < NumFaces; ++i)
        {
            prefilterShader.SetMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            RenderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // generate a 2d LUT from the brdf equations used
    glGenTextures(1, &brdfLUTTexture);

    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, LUTScale, LUTScale, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, LUTScale, LUTScale);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
        GL_TEXTURE_2D, brdfLUTTexture, 0);    
        
    glViewport(0, 0, LUTScale, LUTScale);
    brdfShader.Use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

uint32_t cubeVAO = 0;
uint32_t cubeVBO = 0;
void RenderCube()
{
    if (cubeVAO == 0)
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
        glGenVertexArrays(1, &cubeVAO);
        glBindVertexArray(cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        // fill buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    }
    
    glBindVertexArray(cubeVAO);    
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

uint32_t quadVAO = 0;
uint32_t quadVBO = 0;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        const float quadVertices[] = 
        {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glBindVertexArray(quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::Begin()
{
    int32_t winWidth, winHeight;
    glfwGetWindowSize(Window::GetActive()->m_window, &winWidth, &winHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, winWidth, winHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    texturedShader.Use();
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
}

void Renderer::End()
{
    // draw cubemap background
    const Camera* cam = Camera::GetActive();
    backgroundShader.Use();
    backgroundShader.SetMat4("projection", cam->P);
    backgroundShader.SetMat4("view", cam->V);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
    //glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
    glDisable(GL_CULL_FACE);
    RenderCube();

    // draw UI
    UI::End();
    Window::GetActive()->Swap();
}

void Renderer::DrawTextured(
    Renderer::Buffer            buffer,
    Renderer::Texture           mat,
    Renderer::Texture           norm,
    const Textured::VSUniform&  vsuni,
    const Textured::FSUniform&  fsuni)
{
    texturedShader.SetVec3("Eye", fsuni.Eye);
    texturedShader.SetVec3("LightDir", fsuni.LightDir);
    texturedShader.SetFloat("LightRad", fsuni.LightRad);
    texturedShader.SetMat4("MVP", vsuni.MVP);
    texturedShader.SetMat4("M", vsuni.M);
    texturedShader.SetVec3("Pal0", fsuni.Pal0);
    texturedShader.SetVec3("Pal1", fsuni.Pal1);
    texturedShader.SetVec3("Pal2", fsuni.Pal2);
    texturedShader.SetFloat("PalCenter", fsuni.PalCenter);
    texturedShader.SetFloat("RoughnessOffset", fsuni.RoughnessOffset);
    texturedShader.SetFloat("MetalnessOffset", fsuni.MetalnessOffset);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, mat.id);
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, norm.id);

    glBindVertexArray(buffer.id);
    glDrawElements(GL_TRIANGLES, buffer.count, GL_UNSIGNED_INT, 0);
}

Renderer::Buffer Renderer::CreateBuffer(const Renderer::BufferDesc& desc)
{
    Buffer buffer;
    buffer.count = desc.elementCount;
    glGenVertexArrays(1, &buffer.id);
    glBindVertexArray(buffer.id);

    uint32_t vbo;
    if(desc.vertexData)
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, desc.vertexBytes, desc.vertexData, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
    }

    uint32_t ebo;
    if(desc.indexData)
    {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc.indexBytes, desc.indexData, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    return buffer;
}
void Renderer::DestroyBuffer(Renderer::Buffer buffer)
{
    glDeleteVertexArrays(1, &buffer.id);
}
Renderer::Texture Renderer::CreateTexture(const Renderer::TextureDesc& desc)
{
    Texture texture;
    glGenTextures(1, &texture.id);

    uint32_t glFormat;
    uint32_t glComponents;
    uint32_t glElementType;
    size_t elementSize;
    switch(desc.format)
    {
        case RGBA8:
        glFormat = GL_RGBA8;
        glComponents = GL_RGBA;
        glElementType = GL_UNSIGNED_BYTE;
        elementSize = sizeof(uint8_t) * 4;
        break;
        case RGBA16F:
        glFormat = GL_RGBA16F;
        glComponents = GL_RGBA;
        glElementType = GL_FLOAT;
        elementSize = sizeof(uint16_t) * 4;
        break;
        case RGBA32F:
        glFormat = GL_RGBA32F;
        glComponents = GL_RGBA;
        glElementType = GL_FLOAT;
        elementSize = sizeof(float) * 4;
        break;
        case R32F:
        glFormat = GL_R32F;
        glComponents = GL_RED;
        glElementType = GL_FLOAT;
        elementSize = sizeof(float) * 1;
        break;
        case RG32F:
        glFormat = GL_RG32F;
        glComponents = GL_RG;
        glElementType = GL_FLOAT;
        elementSize = sizeof(float) * 2;
        break;
        case RGB32F:
        glFormat = GL_RGB32F;
        glComponents = GL_RGB;
        glElementType = GL_FLOAT;
        elementSize = sizeof(float) * 3;
        break;
    }
    uint32_t glWrapType;
    switch(desc.wrapType)
    {
        case Clamp:
        glWrapType = GL_CLAMP_TO_EDGE;
        break;
        case Repeat:
        glWrapType = GL_REPEAT;
        break;
    }
    uint32_t glminFilterType;
    switch(desc.minFilter)
    {
        case Nearest:
        glminFilterType = GL_NEAREST;
        break;
        case Linear:
        glminFilterType = GL_LINEAR;
        break;
        case LinearMipmap:
        glminFilterType = GL_LINEAR_MIPMAP_LINEAR;
        break;
    }
    uint32_t glmagFilterType;
    switch(desc.magFilter)
    {
        case Nearest:
        glmagFilterType = GL_NEAREST;
        break;
        case Linear:
        glmagFilterType = GL_LINEAR;
        break;
    }

    uint32_t glTarget;
    switch(desc.type)
    {
        case Texture1D:
        glTarget = GL_TEXTURE_1D;
        glBindTexture(glTarget, texture.id);
        glTexImage1D(glTarget, 0, glFormat, desc.width, 0, glComponents, glElementType, desc.data);
        break;
        case Texture2D:
        glTarget = GL_TEXTURE_2D;
        glBindTexture(glTarget, texture.id);
        glTexImage2D(glTarget, 0, glFormat, desc.width, desc.height, 0, glComponents, glElementType, desc.data);
        break;
        case Texture3D:
        glTarget = GL_TEXTURE_3D;
        glBindTexture(glTarget, texture.id);
        glTexImage3D(glTarget, 0, glFormat, desc.width, desc.height, desc.layers, 0, glComponents, glElementType, desc.data);
        break;
        case TextureCube:
        glTarget = GL_TEXTURE_CUBE_MAP;
        glBindTexture(glTarget, texture.id);
        const uint8_t* pData = (const uint8_t*)desc.data;
        size_t stride = desc.width * desc.height * elementSize;
        for(uint32_t i = 0; i < NumFaces; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glFormat, desc.width, desc.height, 0, glComponents, glElementType, pData + stride * i);
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
        glTexParameteri(glTarget, wrapDims[dim], glWrapType);
    }

    glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, glminFilterType); 
    glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, glmagFilterType);

    if(desc.minFilter == LinearMipmap)
    {
        glGenerateMipmap(glTarget);
    }

    return texture;
}
void Renderer::DestroyTexture(Renderer::Texture texture)
{
    glDeleteTextures(1, &texture.id);
}
void Renderer::Shutdown()
{
    
}