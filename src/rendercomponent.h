#pragma once

#include "component.h"
#include "linmath.h"
#include "pipeline.h"

struct RenderComponent
{
    mat4            m_matrix;
    slot            m_buffer;
    slot            m_normal;
    slot            m_material;
    PipelineType    m_pipeline;

    static const ComponentType ms_type = CT_Render;
};

