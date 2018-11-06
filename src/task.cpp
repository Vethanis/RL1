#include "task.h"

Thread          TaskManager::threads[4];
Semaphore       TaskManager::sema;
Array<Task>     TaskManager::tasks[CT_Count];
uint64_t        TaskManager::duration;
ComponentType    TaskManager::curspace;
