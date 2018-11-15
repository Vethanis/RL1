#include "update.h"

#include <math.h>
#include "component.h"
#include "sokol_time.h"

void Update(float t, float dt)
{
    for(const slot* s = Components::begin(); s != Components::end(); ++s)
    {
        TransformComponent* xform = Components::Get<TransformComponent>(*s);
        if(!xform)
        {
            continue;
        }

        xform->m_rotation *= glm::angleAxis(glm::radians(dt * 100.0f), vec3(0.0f, 1.0f, 0.0f));
        xform->m_position.y += dt * sinf(t * 5.0f);
        xform->m_scale += dt * cosf(t * 5.0f);
    }
}
