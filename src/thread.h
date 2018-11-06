#pragma once

struct Thread
{
    Thread();
    Thread(void (*fn)(void*), void* data);
    void Join();
};
