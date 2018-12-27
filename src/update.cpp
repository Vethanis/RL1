#include "update.h"

#include "component.h"
#include "physics.h"
#include "uidraw.h"
#include "allocator.h"

void Update(float t, float dt)
{
    Allocator::Update();
    UIBegin();
    Physics::Update(dt);
}
