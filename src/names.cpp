#include "names.h"
#include "dict1d.h"

def usize TextLen = 64;

struct Text
{
    char value[TextLen];
};

static Dict1D<u64, Text> ms_dict;

namespace Names
{
    bool Add(u64 hash, cstr name)
    {
        Text text;
        {
            u32 i = 0;
            let len = Min(strlen(name), TextLen - 1u);
            for(; i < len; ++i)
            {
                text.value[i] = name[i];
            }
            text.value[i] = 0;
        }
        return ms_dict.add(hash, text);
    }
    bool Remove(u64 hash)
    {
        return ms_dict.remove(hash);
    }
    cstr Lookup(u64 hash)
    {
        if(let txt = ms_dict.get(hash))
        {
            return txt->value;
        }
        return NullName;
    }
};
