#include "camera.h"

static Camera* ms_active = nullptr;

Camera* Camera::GetActive()
{
    return ms_active;
}

void Camera::SetActive(Camera* cam)
{
    ms_active = cam;
}
