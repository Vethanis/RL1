#include "system.h"

#include <stdio.h>

#include "lang.h"
#include "templates.h"
#include "ai.h"
#include "allocs.h"
#include "audio.h"
#include "control.h"
#include "ctrlbinding.h"
#include "ecs.h"
#include "imvis.h"
#include "physics.h"
#include "renderer.h"
#include "ui.h"
#include "window.h"
#include "sokol_time.h"

// ----------------------------------------------------------------------------

using SystemInitFn      = void(*)(void);
using SystemUpdateFn    = void(*)(void);
using SystemShutdownFn  = void(*)(void);
using ImVisUpdateFn     = void(*)(void);

namespace Systems
{
    static void Init();
    static void Update();
    static void Shutdown();
    static void ImVisChildren();

    static void ImVisUpdate();
};

// ----------------------------------------------------------------------------

def ImVisUpdateFn SystemImVis[] =
{
    ECS::ImVisUpdate,
    Ctrl::ImVisUpdate,
    Systems::ImVisUpdate,
};

def cstr SystemImVisNames[] =
{
    "ECS",
    "Ctrl",
    "Systems",
};
CountAssertEQ(SystemImVis, SystemImVisNames);

static bool ms_imvisEnables[CountOf(SystemImVis)];

// ----------------------------------------------------------------------------

def SystemInitFn SystemInits[] =
{
    LinAlloc::Init,
    ECS::Init,
    Window::Init,
    Ctrl::Init,
    Audio::Init,
    Renderer::Init,
    UI::Init,
    Physics::Init,
    CtrlBinding::Init,
    AI::Init,
};

def cstr SystemInitNames[] =
{
    "LinAlloc",
    "ECS",
    "Window",
    "Ctrl",
    "Audio",
    "Renderer",
    "UI",
    "Physics",
    "CtrlBinding",
    "AI",
};
CountAssertEQ(SystemInits, SystemInitNames);

// ----------------------------------------------------------------------------

def SystemUpdateFn SystemUpdates[] =
{
    LinAlloc::Update,
    UI::Begin,
    Ctrl::Update,
// -------------------
    CtrlBinding::Update,
    AI::Update,
    ECS::Update,
    Physics::Update,
// -------------------
    Audio::Update,
    Renderer::Update,
    Systems::ImVisChildren,
    UI::End,
    Window::Update,
};

def cstr SystemUpdateNames[] =
{
    "LinAlloc",
    "UI",
    "Ctrl",
    "CtrlBinding",
    "AI",
    "ECS",
    "Physics",
    "Audio",
    "Renderer",
    "ImVis",
    "UI",
    "Window",
};
CountAssertEQ(SystemUpdates, SystemUpdateNames);

// ----------------------------------------------------------------------------

def SystemShutdownFn SystemShutdowns[] =
{
    AI::Shutdown,
    CtrlBinding::Shutdown,
    Physics::Shutdown,
    UI::Shutdown,
    Renderer::Shutdown,
    Audio::Shutdown,
    Ctrl::Shutdown,
    Window::Shutdown,
    ECS::Shutdown,
    LinAlloc::Shutdown,
};

def cstr SystemShutdownNames[] =
{
    "AI",
    "CtrlBinding",
    "Physics",
    "UI",
    "Renderer",
    "Audio",
    "Ctrl",
    "Window",
    "ECS",
    "LinAlloc",
};
CountAssertEQ(SystemShutdowns, SystemShutdownNames);

// ----------------------------------------------------------------------------

static u64  ms_initTics[CountOf(SystemInits)];
static u64  ms_updateTics[CountOf(SystemUpdates)];
static u64  ms_shutdownTics[CountOf(SystemShutdowns)];
static u64  ms_imvisTics[CountOf(SystemImVis)];

// ----------------------------------------------------------------------------

def cstr PhaseNames[] =
{
    "Init",
    "Update",
    "ImVis",
};

def cstr const * PhaseSubsystemNames[] =
{
    SystemInitNames,
    SystemUpdateNames,
    SystemImVisNames,
};
CountAssertEQ(PhaseNames, PhaseSubsystemNames);

def u32 PhaseSubsystemCounts[] =
{
    CountOf(SystemInits),
    CountOf(SystemUpdates),
    CountOf(SystemImVis),
};
CountAssertEQ(PhaseNames, PhaseSubsystemCounts);

def const u64* PhaseSubsystemTics[] =
{
    ms_initTics,
    ms_updateTics,
    ms_imvisTics,
};
CountAssertEQ(PhaseNames, PhaseSubsystemTics);

// ----------------------------------------------------------------------------

namespace Systems
{
    static u64  ms_laptime;

    void Quit()
    {
        Window::SetShouldClose(true);
    }
    void Run()
    {
        stm_setup();

        Systems::Init();
        while(Window::IsOpen())
        {
            Systems::Update();
        }
        Systems::Shutdown();
    }

    static void Init()
    {
        u64 now = stm_now();
        for(u32 i = 0; i < CountOf(SystemInits); ++i)
        {
            SystemInits[i]();
            ms_initTics[i] = stm_laptime(&now);
        }
    }

    static void Update()
    {
        u64 now = stm_now();
        for(u32 i = 0; i < CountOf(SystemUpdates); ++i)
        {
            SystemUpdates[i]();
            ms_updateTics[i] = stm_laptime(&now);
        }

        let after = ms_laptime;
        ms_laptime = now;

        for(u32 i = 0; i < CountOf(SystemImVis); ++i)
        {
            let chan = Channel(Key_F1 + i);
            Ctrl::Event evt;
            if(Ctrl::GetAfter(chan, after, &evt))
            {
                if(evt.value == 1.0f)
                {
                    ms_imvisEnables[i] = !ms_imvisEnables[i];
                }
            }
        }
    }

    static void Shutdown()
    {
        u64 now = stm_now();
        for(u32 i = 0; i < CountOf(SystemShutdowns); ++i)
        {
            SystemShutdowns[i]();
            ms_shutdownTics[i] = stm_laptime(&now);
        }

        f64 sum = 0.0;
        for(let tics : ms_shutdownTics)
        {
            sum += stm_ms(tics);
        }
        let inv = 100.0f / sum;

        printf("Shutdown\n");
        printf("Total: %gms\n", sum);
        printf("%-12s | %-12s | %-12s\n", "Name", "ms", "Percent");
        for(u32 i = 0; i < CountOf(SystemShutdowns); ++i)
        {
            let name = SystemShutdownNames[i];
            let ms = stm_ms(ms_shutdownTics[i]);
            let pct = ms * inv;
            printf("%-12s | %-12g | %-12g\n", name, ms, pct);
        }
    }

    static void ImVisChildren()
    {
        u64 now = stm_now();
        for(u32 i = 0; i < CountOf(SystemImVis); ++i)
        {
            if(ms_imvisEnables[i])
            {
                SystemImVis[i]();
            }
            ms_imvisTics[i] = stm_laptime(&now);
        }
    }

    static void ImVisUpdate()
    {
        ImGui::SetNextWindowSize({ 400, 400 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Systems");

        for(u32 phase = 0; phase < CountOf(PhaseNames); ++phase)
        {
            let name        = PhaseNames[phase];
            let subTics     = PhaseSubsystemTics[phase];
            let subNames    = PhaseSubsystemNames[phase];
            let subCount    = PhaseSubsystemCounts[phase];

            if(ImGui::CollapsingHeader(name))
            {
                f32 sum = 0.0f;
                for(u32 i = 0; i < subCount; ++i)
                {
                    sum += stm_ms(subTics[i]);
                }
                let inv = 100.0f / sum;

                ImGui::Text("Total: %gms", sum);

                ImGui::Columns(3);
                ImGui::Text("Name");    ImGui::NextColumn();
                ImGui::Text("ms");      ImGui::NextColumn();
                ImGui::Text("%%");      ImGui::NextColumn();

                for(u32 i = 0; i < subCount; ++i)
                {
                    let ms = stm_ms(subTics[i]);
                    ImGui::Text("%s", subNames[i]); ImGui::NextColumn();
                    ImGui::Text("%g", ms);          ImGui::NextColumn();
                    ImGui::Text("%g", ms * inv);    ImGui::NextColumn();
                }
                ImGui::Columns();
            }
        }
        ImGui::End();
    }
};
