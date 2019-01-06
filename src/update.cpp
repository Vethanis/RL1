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
        int32_t i = 0;
        yawPitch[i++] = Control::RegisterAxis("Yaw");
        yawPitch[i++] = Control::RegisterAxis("Pitch");
        i = 0;
        movement[i++] = Control::RegisterAction("Right");
        movement[i++] = Control::RegisterAction("Left");
        movement[i++] = Control::RegisterAction("Up");
        movement[i++] = Control::RegisterAction("Down");
        movement[i++] = Control::RegisterAction("Forward");
        movement[i++] = Control::RegisterAction("Backward");

        i = 0;
        Control::BindToAxis(yawPitch[i++], Control::AL_LARGEST, Control::MA_Cursor_X);
        Control::BindToAxis(yawPitch[i++], Control::AL_LARGEST, Control::MA_Cursor_Y);

        i = 0;
        Control::BindToAction(movement[i++], Control::BL_OR, Control::K_D);
        Control::BindToAction(movement[i++], Control::BL_OR, Control::K_A);
        Control::BindToAction(movement[i++], Control::BL_OR, Control::K_SPACE);
        Control::BindToAction(movement[i++], Control::BL_OR, Control::K_LEFT_SHIFT);
        Control::BindToAction(movement[i++], Control::BL_OR, Control::K_W);
        Control::BindToAction(movement[i++], Control::BL_OR, Control::K_S);
    }
    {
        Camera* cam = Camera::GetActive();
        float yaw = Control::GetAxisControlDelta(yawPitch[0]);
        float pitch = Control::GetAxisControlDelta(yawPitch[1]);

        vec3 dv = vec3(0.0f);
        for(int32_t i = 0; i < NELEM(movement); ++i)
        {
            uint8_t state = Control::GetActionState(movement[i]);
            float sign = (i & 1) ? -1.0f : 1.0f;
            dv[i >> 1] += state ? sign * dt : 0.0f;
        }

        cam->move(dv);
        cam->pitch(pitch * dt);
        cam->yaw(yaw * dt);
    }
    Physics::Update(dt);
}
