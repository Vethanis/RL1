#include "resourcebase.h"

#include "task.h"
#include "macro.h"

ResourceBase* ResourceBase::s_resources[RT_Count];
uint64_t      ResourceBase::s_durations[RT_Count];

void ResourceBase::TaskAll(TaskManager& manager)
{
    for(int32_t i = 0; i < NELEM(s_resources); ++i)
    {
        ResourceBase* base = s_resources[i];
        base->CreateTasks(manager);
        manager.SetDuration(s_durations[i]);
        manager.Start();
    }
}

// SETUP -------------------------------------------------------

#include "resource.h"

#include "buffer.h"
#include "image.h"
#include "transform.h"

static Resource<Buffer>         s_buf;
static Resource<Image>          s_img;
static ResourceSimple<mat4>     s_mat;

void ResourceBase::Setup()
{
    Set(RT_Buffer,      &s_buf);
    Set(RT_Image,       &s_img);
    Set(RT_Transform,   &s_mat);
}
