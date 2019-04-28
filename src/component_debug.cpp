#include "component_debug.h"
#include "component_types.h"
#include "names.h"
#include "imvis.h"

using ComponentImVisFn  = void(*)(CComponentData component);

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
    let pos = Cast<Position>(x)->value;
    ImVisFloat3(pos);
}

static void ImVisOrientation(CComponentData x)
{
    let orien = Cast<Orientation>(x);
    let f = orien->forward;
    let u = orien->up;
    ImGui::Text(
        "%g %g %g, %g %g %g",
        f.x, f.y, f.z,
        u.x, u.y, u.z);
}

static void ImVisScale(CComponentData x)
{
    let scale = Cast<Scale>(x)->value;
    ImVisFloat3(scale);
}

static void ImVisVelocity(CComponentData x)
{
    let vel = Cast<Velocity>(x)->value;
    ImVisFloat3(vel);
}

static void ImVisMass(CComponentData x)
{
    let mass = Cast<Mass>(x)->value;
    ImVisF32(mass);
}

static void ImVisNameHash(CComponentData x)
{
    let hash = Cast<NameHash>(x)->value;
    ImVisCstr(Names::Lookup(hash));
}

static void ImVisDrawInfo(CComponentData x)
{
    let info = Cast<DrawInfo>(x);
    ImGui::Text("%u %g %g", info->id, info->phase, info->hz);
}

static void ImVisHealth(CComponentData x)
{
    let health = ToType<Health>(x)[0].value;
    ImVisF32(health);
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
