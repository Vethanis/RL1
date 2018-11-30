#include "update.h"

#include "component.h"
#include "physics.h"
#include "uidraw.h"

void Update(float t, float dt)
{
    UIBegin();
    Physics::Update(dt);
}
