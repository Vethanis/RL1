#pragma once

#include <stdint.h>
#include "slot.h"
#include "array.h"

namespace ScriptTypes
{
    enum Constants : int32_t
    {
        PCStackSize = 128,
        StackSize   = 1024,
        MaxHandles  = 64,
    };

    enum Op : uint8_t
    {
        NOP = 0,// do nothing
        LOAD,   // pop 1, push stack[a]
        STORE,  // pop 2, stack[a] = b
        PUSH,   // read 1, push a
        POP,    // pop 1
        DUP,    // pop 1, push a, push a
        SWAP2,  // pop 2, push b, a
        SWAP3,  // pop 3, push c, b, a
        PROC,   // pop 1, push pc onto pc stack, set pc to a
        RET,    // pop pc stack, pc = a
        CALL,   // pop 1, pop N, push 1; funcID = a, args = b.., push retval
        ADD,   // pop 2, push a + b
        SUB,   // pop 2, push a - b
        MUL,   // pop 2, push a * b
        DIV,   // pop 2, push a / b, push a % b
        NEG,   // pop 1, push -1
        SHL,    // pop 2, push a << b
        SHR,    // pop 2, push a >> b
        NOT,    // pop 1, push ~a
        AND,    // pop 2, push a & b
        OR,     // pop 2, push a | b
        XOR,    // pop 2, push a ^ b
        NOR,    // pop 2, push ~(a | b)
        NAND,   // pop 2, push ~(a & b)
        JMP,    // pop 1, pc = a
        JZ,    // pop 2, if a == 0: pc = b
        JNZ,   // pop 2, if a != 0: pc = b
        JE,    // pop 3, if a == b: pc = c
        JNE,   // pop 3, if a != b pc = c
        JL,    // pop 3, if a < b  pc = c
        JG,    // pop 3, if a > b  pc = c
        JLE,   // pop 3, if a <= b pc = c
        JGE,   // pop 3, if a >= b pc = c
        SIN,   // pop 1, push sin(a)
        COS,   // pop 1, push cos(a)
        TAN,   // pop 1, push tan(a)
        ASIN,  // pop 1, push asin(a)
        ACOS,  // pop 1, push acos(a)
        ATAN,  // pop 2, push atan2(a, b)
        MIN,   // pop 2, push Min(a, b)
        MAX,   // pop 2, push Max(a, b)
        CLAMP, // pop 3, push Clamp(a, b, c)
        MIX,   // pop 3, push Lerp(a, b, c)
        YIELD,  // pop 1; yield a times
        KILL,   // end execution and free all resources held
        SLEEP,  // pop 1; hibernate a seconds
    };

    typedef double Value;

    struct NativeID
    {
        int32_t     id;
    };

    struct ResourceHandle
    {
        slot        id;
        NativeID    src;
    };

    typedef FixedArray<int32_t,         PCStackSize>    PCStack;
    typedef FixedArray<Value,           StackSize>      Stack;
    typedef FixedArray<ResourceHandle,  MaxHandles>     ResourceHandles;

    struct NativeExecData
    {
        NativeID            id;
        Stack*              pStack;
        ResourceHandles*    pHandles;
    };

    typedef void (*NativeFunc)(NativeExecData);
    typedef void (*NativeResourceFn)(ResourceHandle);

    struct NativeFuncData
    {
        NativeFunc          func;
        NativeResourceFn    sleeper;
        NativeResourceFn    waker;
        NativeResourceFn    destructor;
    };

    struct CompileResult
    {
        Array<uint8_t> code;
    };

    struct ProcessHandle
    {
        slot id;
    };

    struct ProcessCreateInfo
    {
        const uint8_t*  code;
        int32_t         codeLen;
    };

};