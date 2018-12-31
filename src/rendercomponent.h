#pragma once

#include "component.h"
#include "linmath.h"

enum PipelineType
{
    PT_Textured = 0,
    PT_Flat,
    PT_Count,
};

struct RenderComponent
{
    mat4            m_matrix;
    PipelineType    m_type;
    slot            m_buffer;
    slot            m_normal;
    slot            m_material;

    static const ComponentType ms_type = CT_Render;
};

