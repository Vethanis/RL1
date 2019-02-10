#pragma once

#include "component.h"
#include "linmath.h"
#include "renderer.h"

enum PipelineType
{
    PT_Textured = 0,
    PT_Flat,
    PT_Count,
};

struct RenderComponent
{
    mat4                m_matrix;
    PipelineType        m_type;
    Renderer::Buffer    m_vertices;
    Renderer::Buffer    m_indices;
    Renderer::Texture   m_material;
    Renderer::Texture   m_normal;

    static const ComponentType ms_type = CT_Render;
};

