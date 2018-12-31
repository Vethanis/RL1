#include "update.h"

#include "component.h"
#include "physics.h"
#include "ui.h"
#include "allocator.h"

void Update(float t, float dt)
{
    Allocator::Update();
    UI::Begin(dt);
    Physics::Update(dt);
}
