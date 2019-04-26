#include "component_debug.h"
#include "component_types.h"
#include "imgui.h"

using ComponentImVisFn  = void(*)(CComponentData compData);

static constexpr const char* sc_ComponentNames[] =
{
    "Position",
    "Orientation",
    "Scale",
    "Velocity",
    "Mass",
    "AABB",
    "Sphere",
    "Plane",
    "Name",
    "NameHash",
    "Inventory",
    "Health",
};
CountAssert(sc_ComponentNames, CT_Count);

#define Extract(T, x) ToType<T>(x)[0]

static void ImVisPosition(CComponentData x)
{
    let pos = Extract(Position, x).position;
    ImGui::Text("%-6g %-6g %-6g\n", pos.x, pos.y, pos.z);
}

static void ImVisOrientation(CComponentData x)
{

}

static void ImVisScale(CComponentData x)
{

}

static void ImVisVelocity(CComponentData x)
{

}

static void ImVisMass(CComponentData x)
{

}

static void ImVisAABB(CComponentData x)
{

}

static void ImVisSphere(CComponentData x)
{

}

static void ImVisPlane(CComponentData x)
{

}

static void ImVisName(CComponentData x)
{

}

static void ImVisNameHash(CComponentData x)
{

}

static void ImVisInventory(CComponentData x)
{

}

static void ImVisHealth(CComponentData x)
{

}

static constexpr ComponentImVisFn sc_ComponentImVis[] =
{
    ImVisPosition,
    ImVisOrientation,
    ImVisScale,
    ImVisVelocity,
    ImVisMass,
    ImVisAABB,
    ImVisSphere,
    ImVisPlane,
    ImVisName,
    ImVisNameHash,
    ImVisInventory,
    ImVisHealth,
};
CountAssert(sc_ComponentImVis, CT_Count);

void ImVisComponent(ComponentType type, CComponentData data)
{
    sc_ComponentImVis[type](data);
}

const char* ComponentName(ComponentType type)
{
    return sc_ComponentNames[type];
}
