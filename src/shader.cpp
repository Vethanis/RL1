#include "shader.h"

#include "macro.h"
#include "gen_array.h"

namespace Shaders
{
    gen_array<sg_shader> ms_store;

    slot Create(const sg_shader_desc& desc)
    {
        slot s = ms_store.Create();
        sg_shader* p = ms_store.Get(s);
        *p = sg_make_shader(&desc);
        Assert(p->id != SG_INVALID_ID);
        return s;
    }
    void Destroy(slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        sg_shader* p = ms_store.Get(s);
        sg_destroy_shader(*p);
        ms_store.Destroy(s);
    }
    sg_shader Get(slot s)
    {
        sg_shader id = { SG_INVALID_ID };
        sg_shader* p = ms_store.Get(s);
        return p ? *p : id;
    }
    bool Exists(slot s)
    {
        return ms_store.Exists(s);
    }
};
