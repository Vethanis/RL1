#include "system.h"

#include <stdio.h>

#include "lang.h"
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
#include "sort.h"

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

static bool ms_imvisEnables[LEN(SystemImVis)];

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
    "UIBegin",
    "Ctrl",
    "CtrlBinding",
    "AI",
    "ECS",
    "Physics",
    "Audio",
    "Renderer",
    "ImVis",
    "UIEnd",
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

static f64  ms_initMs[LEN(SystemInits)];
static f64  ms_updateMs[LEN(SystemUpdates)];
static f64  ms_shutdownMs[LEN(SystemShutdowns)];
static f64  ms_imvisMs[LEN(SystemImVis)];

static f32  ms_timeSmooth = 60.0;

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
    LEN(SystemInits),
    LEN(SystemUpdates),
    LEN(SystemImVis),
};
CountAssertEQ(PhaseNames, PhaseSubsystemCounts);

def const f64* PhaseSubsystemMs[] =
{
    ms_initMs,
    ms_updateMs,
    ms_imvisMs,
};
CountAssertEQ(PhaseNames, PhaseSubsystemMs);

// ----------------------------------------------------------------------------

namespace Systems
{
    static u64 ms_laptime;

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
        for(u32 i = 0; i < LEN(SystemInits); ++i)
        {
            SystemInits[i]();
            ms_initMs[i] = stm_ms(stm_laptime(&now));
        }
    }

    static void Update()
    {
        let smoothing = 1.0 / ms_timeSmooth;
        u64 now = stm_now();
        for(u32 i = 0; i < LEN(SystemUpdates); ++i)
        {
            SystemUpdates[i]();
            let ms = stm_ms(stm_laptime(&now));
            ms_updateMs[i] = Lerp(ms_updateMs[i], ms, smoothing);
        }

        let after = ms_laptime;
        ms_laptime = now;

        for(u32 i = 0; i < LEN(SystemImVis); ++i)
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
        constexpr u32 len = LEN(SystemShutdowns);

        u64 now = stm_now();
        for(u32 i = 0; i < len; ++i)
        {
            SystemShutdowns[i]();
            ms_shutdownMs[i] = stm_ms(stm_laptime(&now));
        }

        let sum = Sum(ms_shutdownMs, len);
        let inv = 100.0 / sum;

        u32 order[len];
        IndexFill(order, len);

        Sort(order, len, [&](u32 a, u32 b) -> i32
        {
            let msA = ms_shutdownMs[a];
            let msB = ms_shutdownMs[b];
            if(msA == msB)
            {
                return 0;
            }
            return (msA > msB) ? -1 : 1;
        });

        printf("-----------------------------\n");
        printf("Shutdown: %fms\n", sum);
        printf("-----------------------------\n");
        printf("%-12s | %-5s | %c\n", "Name", "ms", '%');
        printf("-------------|-------|-------\n");
        for(u32 i = 0; i < len; ++i)
        {
            let ind = order[i];
            let name = SystemShutdownNames[ind];
            let ms = ms_shutdownMs[ind];
            let pct = ms * inv;
            printf("%-12s | %05.2f | %05.2f\n", name, ms, pct);
        }
        printf("-------------|-------|-------\n\n");
    }

    static void ImVisChildren()
    {
        let smoothing = 1.0 / ms_timeSmooth;
        u64 now = stm_now();
        for(u32 i = 0; i < LEN(SystemImVis); ++i)
        {
            if(ms_imvisEnables[i])
            {
                SystemImVis[i]();
            }
            let ms = stm_ms(stm_laptime(&now));
            ms_imvisMs[i] = Lerp(ms_imvisMs[i], ms, smoothing);
        }
    }

    static void ImVisUpdate()
    {
        ImGui::SetNextWindowSize({ 300, 650 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Systems");

        ImGui::SliderFloat("Smoothing", &ms_timeSmooth, 1.0f, 240.0f, "%05.2f frames", 3.0f);

        u32 order[MaxA(PhaseSubsystemCounts, LEN(PhaseNames))];

        for(u32 phase = 0; phase < LEN(PhaseNames); ++phase)
        {
            let subMs       = PhaseSubsystemMs[phase];
            let subNames    = PhaseSubsystemNames[phase];
            let subCount    = PhaseSubsystemCounts[phase];
            let sum         = Sum(subMs, subCount);
            let inv         = 100.0 / sum;

            IndexFill(order, subCount);
            Sort(order, subCount, [&](u32 a, u32 b) -> i32
            {
                let msA = subMs[a];
                let msB = subMs[b];
                if(msA == msB)
                {
                    return 0;
                }
                return (msA > msB) ? -1 : 1;
            });

            char hdr[64];
            Format(hdr, "%s: %05.2fms", PhaseNames[phase], sum);

            if(ImGui::CollapsingHeader(hdr, ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Columns(3);
                ImGui::Text("Name");    ImGui::NextColumn();
                ImGui::Text("ms");      ImGui::NextColumn();
                ImGui::Text("%%");      ImGui::NextColumn();
                ImGui::Separator();
                for(u32 i = 0; i < subCount; ++i)
                {
                    let ind = order[i];
                    ImGui::Text("%-12s",  subNames[ind]);     ImGui::NextColumn();
                    ImGui::Text("%05.2f", subMs[ind]);        ImGui::NextColumn();
                    ImGui::Text("%05.2f", subMs[ind] * inv);  ImGui::NextColumn();
                }
                ImGui::Columns();
            }
        }
        ImGui::End();
    }
};
