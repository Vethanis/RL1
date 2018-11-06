#include "name.h"

Names Names::ms_names[NS_Count];

int32_t Names::Add(const char* name)
{
    Text& t = m_text.grow();
    memset(&t, 0, sizeof(Text));
    uint64_t hash = m_hashes.grow();
    hash = Fnv64(name);

    int32_t i = 0;
    const int32_t len = NELEM(t.data) - 1;
    for(; i < len && name[i]; ++i)
    {
        t.data[i] = name[i];
    }
    t.data[i] = '\0';

    return m_text.count() - 1;
}