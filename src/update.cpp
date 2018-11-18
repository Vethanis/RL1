#include "update.h"

#include "component.h"
#include "physics.h"

void Update(float t, float dt)
{    
    Physics::Update(dt);
}
