
#include "system.h"
#include "task.h"

int main()
{
    const TaskMgrDesc desc = { 8, 8 };
    TaskMgrInit(&desc);

    Systems::Run();

    TaskMgrShutdown();
}
