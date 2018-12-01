#include "hashstring.h"

#include "name.h"

slot SlotStringConstructor(Namespace ns, const char* x)
{
    return Names::GetSpace(ns).Create(x);
}

slot SlotStringLookup(Namespace ns, Hash hash)
{
    return Names::GetSpace(ns).Find(hash);
}

const char* SlotStringGetString(Namespace ns, slot s)
{
    return Names::GetSpace(ns)[s];
}
