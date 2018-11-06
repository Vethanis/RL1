#include "component.h"

#include "macro.h"
#include "resource.h"
#include "slotsresource.h"
#include "loader.h"

#include "buffer.h"
#include "image.h"
#include "transform.h"

// ---------------------------------------------------------

Loader<Buffer>              s_buff;
Loader<Image>               s_img;
Resource<Transform>         s_xform;

// ---------------------------------------------------------

ResourceBase* s_resources[CT_Count] = 
{
    &s_buff,
    &s_img,
    &s_xform,
};

// ---------------------------------------------------------

#define GetMacro(T, name) \
    template<>  \
    T* Component::Get(slot s) { return (T*)(name.Get(s)); }

GetMacro(Buffer,    s_buff)
GetMacro(Image,     s_img)
GetMacro(Transform, s_xform)

// ---------------------------------------------------------

bool Component::HasLoader(ComponentType type)
{
    return s_resources[type]->IsLoader();
}

void Component::Add(ComponentType type, slot s)
{
    s_resources[type]->Add(s);
}

void Component::Add(ComponentType type, slot s, int32_t name, uint8_t method)
{
    Assert(HasLoader(type));
    LoaderBase* loader = static_cast<LoaderBase*>(s_resources[type]);
    loader->Add(s, name, method);
}

void Component::Remove(ComponentType type, slot s)
{
    s_resources[type]->Remove(s);
}

bool Component::Exists(ComponentType type, slot s)
{
    return s_resources[type]->Exists(s);
}

void Component::Update(ComponentType type)
{
    Assert(HasLoader(type));
    LoaderBase* loader = static_cast<LoaderBase*>(s_resources[type]);
    loader->Update();
}

void Component::DoTasks(ComponentType type)
{
    Assert(HasLoader(type));
    LoaderBase* loader = static_cast<LoaderBase*>(s_resources[type]);
    loader->DoTasks();
}

// ---------------------------------------------------------

void Component::UpdateAll()
{
    for(int32_t i = 0; i < CT_Count; ++i)
    {
        ComponentType type = (ComponentType)i;
        if(HasLoader(type))
        {
            DoTasks(type);
            Update(type);
        }
    }
}

void Component::RemoveAll(slot s)
{
    for(int32_t i = 0; i < CT_Count; ++i)
    {
        if(Exists((ComponentType)i, s))
        {
            Remove((ComponentType)i, s);
        }
    }
}

// ---------------------------------------------------------
