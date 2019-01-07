#include "physics.h"

#include "macro.h"
#include "gen_array.h"
#include "blockalloc.h"
#include "rendercomponent.h"

namespace Physics
{
    void Init()
    {

    }
    void Update(float dt)
    {
        for(const slot* s = Components::begin(); s != Components::end(); ++s)
        {
            PhysicsComponent* pc = Components::Get<PhysicsComponent>(*s);
            RenderComponent* rc = Components::Get<RenderComponent>(*s);
            if(!pc | !rc)
            {
                continue;
            }

            rc->m_matrix = pc->GetTransform();
        }
    }
    void Shutdown()
    {

    }
    void* Create(float mass, const vec3& position, const vec3& extent)
    {
        return nullptr;
    }
    void Destroy(void* body)
    {
        if(body)
        {

        }
    }
};
