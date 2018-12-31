#pragma once

#include "component.h"
#include "linmath.h"

struct RenderComponent
{
    mat4            m_matrix;
    slot            m_buffer;
    slot            m_normal;
    slot            m_material;

    static const ComponentType ms_type = CT_Render;
};

