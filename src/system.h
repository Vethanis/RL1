#pragma once

#include "ai.h"
#include "allocs.h"
#include "audio.h"
#include "control.h"
#include "ecs.h"
#include "physics.h"
#include "renderer.h"
#include "ui.h"
#include "window.h"

using SystemInitFn      = void(*)(void);
using SystemUpdateFn    = void(*)(void);
using SystemShutdownFn  = void(*)(void);

static constexpr SystemInitFn sc_SystemInits[] =
{
    LinAlloc::Init,
    ECS::Init,
    Window::Init,
    Ctrl::Init,
    Audio::Init,
    Renderer::Init,
    UI::Init,
    Physics::Init,
    AI::Init,
};

static constexpr SystemUpdateFn sc_SystemUpdates[] =
{
    LinAlloc::Update,
    UI::Begin,
    Ctrl::Update,
// -------------------
    AI::Update,
    ECS::Update,
    Physics::Update,
// -------------------
    Audio::Update,
    Renderer::Update,
    UI::End,
    Window::Update,
};

static constexpr SystemShutdownFn sc_SystemShutdowns[] =
{
    AI::Shutdown,
    Physics::Shutdown,
    UI::Shutdown,
    Renderer::Shutdown,
    Audio::Shutdown,
    Ctrl::Shutdown,
    Window::Shutdown,
    ECS::Shutdown,
    LinAlloc::Shutdown,
};
