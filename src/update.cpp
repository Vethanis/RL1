#include "update.h"

#include "component.h"
#include "physics.h"
#include "ui.h"
#include "allocator.h"
#include "control.h"
#include "camera.h"

int32_t yawPitch[2];
int32_t movement[6];
bool haveControls = false;

void Update(float t, float dt)
{
    Allocator::Update();
    UI::Begin(dt);
    Control::Update(dt);
    if(!haveControls)
    {
        haveControls = true;
        yawPitch[0] = Control::RegisterAxis("Yaw");
        yawPitch[1] = Control::RegisterAxis("Pitch");
        movement[0] = Control::RegisterAction("Forward");
        movement[1] = Control::RegisterAction("Backward");
        movement[2] = Control::RegisterAction("Left");
        movement[3] = Control::RegisterAction("Right");
        movement[4] = Control::RegisterAction("Up");
        movement[5] = Control::RegisterAction("Down");

        Control::BindToAxis(yawPitch[0], Control::AL_LARGEST, Control::MA_Cursor_X);
        Control::BindToAxis(yawPitch[1], Control::AL_LARGEST, Control::MA_Cursor_Y);

        Control::BindToAction(movement[0], Control::BL_OR, Control::K_W);
        Control::BindToAction(movement[1], Control::BL_OR, Control::K_S);
        Control::BindToAction(movement[2], Control::BL_OR, Control::K_A);
        Control::BindToAction(movement[3], Control::BL_OR, Control::K_D);
        Control::BindToAction(movement[4], Control::BL_OR, Control::K_SPACE);
        Control::BindToAction(movement[5], Control::BL_OR, Control::K_LEFT_SHIFT);
    }
    {
        Camera* cam = Camera::GetActive();
        float yaw = Control::GetAxisControlDelta(yawPitch[0]);
        float pitch = Control::GetAxisControlDelta(yawPitch[1]);
        uint8_t states[6];
        MemZero(states);
        for(int32_t i = 0; i < NELEM(movement); ++i)
        {
            states[i] = Control::GetActionState(movement[i]);
        }
        vec3 dv = vec3(0.0f);
        dv.z += states[0] ? dt : 0.0f;
        dv.z -= states[1] ? dt : 0.0f;
        dv.x += states[2] ? dt : 0.0f;
        dv.x -= states[3] ? dt : 0.0f;
        dv.y += states[4] ? dt : 0.0f;
        dv.y -= states[5] ? dt : 0.0f;

        cam->move(dv);
        cam->pitch(pitch * dt);
        cam->yaw(yaw * dt);
    }
    Physics::Update(dt);
}
