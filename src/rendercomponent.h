#pragma once

#include "component.h"
#include "linmath.h"
#include "pipeline.h"

struct RenderComponent
{
    mat4            m_matrix;
    PipelineType    m_pipeline;
    slot            m_buffer;
    slot            m_normal;
    slot            m_material;

    static const ComponentType ms_type = CT_Render;
};

