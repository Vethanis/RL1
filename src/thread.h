#pragma once

struct Thread
{
    Thread(void (*fn)(void*), void*);
    void Join();
};