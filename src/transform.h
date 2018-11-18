#pragma once

#include "linmath.h"
#include "component.h"

typedef mat4 Transform;

struct TransformComponent
{
    mat4  m_matrix;

    static const ComponentType ms_type = CT_Transform;
};
