#include "component_debug.h"
#include "component_types.h"
#include "names.h"

#include "imgui.h"

using ComponentImVisFn  = void(*)(CComponentData compData);

// ----------------------------------------------------------------------------

static constexpr const char* sc_ComponentNames[] =
{
    "Position",
    "Orientation",
    "Scale",
    "Velocity",
    "Mass",
    "NameHash",
    "DrawInfo",
    "Health",
};
CountAssert(sc_ComponentNames, CT_Count);

const char* ComponentName(ComponentType type)
{
    return sc_ComponentNames[type];
}

// ----------------------------------------------------------------------------

static void ImVisPosition(CComponentData x)
{
    let pos = ToType<Position>(x)[0].value;
    ImGui::Text("%g %g %g", pos.x, pos.y, pos.z);
}

static void ImVisOrientation(CComponentData x)
{
    let orien = ToType<Orientation>(x)[0];
    let dir = orien.forward;
    let up = orien.up;
    ImGui::Text("%g %g %g | ", dir.x, dir.y, dir.z);
    ImGui::SameLine();
    ImGui::Text("%g %g %g", up.x, up.y, up.z);
}

static void ImVisScale(CComponentData x)
{
    let scale = ToType<Scale>(x)[0].value;
    ImGui::Text("%g %g", scale.x, scale.y);
}

static void ImVisVelocity(CComponentData x)
{
    let vel = ToType<Velocity>(x)[0].value;
    ImGui::Text("%g %g %g", vel.x, vel.y, vel.z);
}

static void ImVisMass(CComponentData x)
{
    let mass = ToType<Mass>(x)[0].value;
    ImGui::Text("%g", mass);
}

static void ImVisNameHash(CComponentData x)
{
    let hash = ToType<NameHash>(x)[0].value;
    ImGui::Text("%s", Names::Lookup(hash));
}

static void ImVisDrawInfo(CComponentData x)
{
    let info = ToType<DrawInfo>(x)[0];
    ImGui::Text("%u %g %g", info.id, info.phase, info.hz);
}

static void ImVisHealth(CComponentData x)
{
    let health = ToType<Health>(x)[0].value;
    ImGui::Text("%g", health);
}

// ----------------------------------------------------------------------------

static constexpr ComponentImVisFn sc_ComponentImVis[] =
{
    ImVisPosition,
    ImVisOrientation,
    ImVisScale,
    ImVisVelocity,
    ImVisMass,
    ImVisNameHash,
    ImVisDrawInfo,
    ImVisHealth,
};
CountAssert(sc_ComponentImVis, CT_Count);

void ImVisComponent(ComponentType type, CComponentData data)
{
    sc_ComponentImVis[type](data);
}

// ----------------------------------------------------------------------------
