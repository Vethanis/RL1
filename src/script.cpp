#include "script.h"

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "array.h"
#include "gen_array.h"
#include "native.h"
#include "sokol_time.h"

namespace Script
{
    using namespace ScriptTypes;
// stack positional args order:
/*
    bottom   ... top
    0, 1, 2, ...
    [a][b][c]
    pop -> c
    pop -> b
    pop -> a
    
    []
    push("fart")
    ['f']['a']['r']['t']
    pop -> 't'
    pop -> 'r'
    pop -> 'a'
    pop -> 'f'
*/
// for engine calls, any signature that returns a slot or frees a slot must be added to handles list

    struct Process
    {
        int32_t             m_pc;
        int32_t             m_yield;
        int32_t             m_sleepSecs;
        int32_t             m_codeLen;
        uint64_t            m_sleepBegin;
        const uint8_t*      m_code;
        PCStack             m_pcStack;
        Stack               m_stack;
        ResourceHandles     m_handles;

        void Init(const ProcessCreateInfo& info)
        {
            MemZero(*this);
            m_code = info.code;
            m_codeLen = info.codeLen;
        }
        void Destroy()
        {
            for(ResourceHandle hdl : m_handles)
            {
                Native::Destroy(hdl);
            }
        }
        inline void PrintStack(FILE* file)
        {
            for(Value v : m_stack)
            {
                fprintf(file, "[%g]", v);
            }
            fprintf(file, "\n");
        }
        inline Value Pop()
        {
            Value v = m_stack.back();
            m_stack.pop();
            return v;
        }
        inline void Push(Value v)
        {
            m_stack.grow() = v;
        }
        inline Value Read(int32_t& pc) const
        {
            Value v;
            const uint8_t* addr = &m_code[pc];
            memcpy(&v, addr, sizeof(Value));
            pc += sizeof(Value);
            return v;
        }
        inline void Wake()
        {
            if(m_sleepSecs > 10)
            {
                // deserialize this struct from file
                {

                }
                for(ResourceHandle hdl : m_handles)
                {
                    Native::Wake(hdl);
                }
            }
        }
        inline void Sleep()
        {
            if(m_sleepSecs > 10)
            {
                for(ResourceHandle hdl : m_handles)
                {
                    Native::Sleep(hdl);
                }
                // serialize this struct to file
                {

                }
            }
        }
        void Run(double usecs);
    };  // Process

    void Process::Run(double usecs)
    {
        if(m_yield > 0)
        {
            --m_yield;
            return;
        }
        if(m_sleepSecs > 0)
        {
            uint64_t ticks = stm_since(m_sleepBegin);
            double secs = stm_sec(ticks);
            if((int32_t)secs >= m_sleepSecs)
            {
                Wake();
                m_sleepSecs = 0;
            }
            else
            {
                m_yield += 60;
                return;
            }
        }

        const uint64_t runBegin = stm_now();
        uint8_t numOps = 0;

        for(int32_t pc = m_pc; pc < m_codeLen;)
        {
            ++numOps;
            if(!numOps)
            {
                if(stm_us(stm_diff(stm_now(), runBegin)) >= usecs)
                {
                    m_pc = pc;
                    return;
                }
            }

            uint8_t value = m_code[pc];
            ++pc;
            switch(value)
            {
                case NOP:   // do nothing
                {

                }
                break;
                case LOAD:   // pop 1, push stack[a]
                {
                    Value a = Pop();
                    Value b = m_stack[(int32_t)a];
                    Push(b);
                }
                break;
                case STORE:  // pop 2, stack[a] = b
                {
                    Value b = Pop();
                    Value a = Pop();
                    m_stack[(int32_t)a] = b;
                }
                break;
                case PUSH:  // read 1, push a
                {
                    Value a = Read(pc);
                    Push(a);
                }
                break;
                case POP:    // pop 1
                {
                    Pop();
                }
                break;
                case DUP:   // pop 1, push a, push a
                {
                    Value a = Pop();
                    Push(a);
                    Push(a);
                }
                break;
                case SWAP2:   // pop 2, push b, push a
                {
                    Value b = Pop();
                    Value a = Pop();
                    Push(b);
                    Push(a);
                }
                break;
                case SWAP3:  // pop 3, push c, b, a
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    Push(c);
                    Push(b);
                    Push(a);
                }
                break;
                case PROC:   // pop 1, push pc onto pc stack, set pc to a
                {
                    m_pcStack.grow() = pc;
                    Value a = Pop();
                    pc = (int32_t)a;
                }
                break;
                case RET:    // pop pc stack, pc = a
                {
                    pc = m_pcStack.back();
                    m_pcStack.pop();
                }
                break;
                case CALL:   // pop 1, pop N, push 1; funcID = a, args = b.., push retval
                {
                    Value a = Pop();
                    NativeID fid;
                    fid.id = (int32_t)a;

                    NativeExecData ned;
                    MemZero(ned);
                    ned.id = fid;
                    ned.pHandles = &m_handles;
                    ned.pStack = &m_stack;
                    Native::Exec(ned);
                }
                break;
                case ADD:   // pop 2, push a + b
                {
                    Value b = Pop();
                    Value a = Pop();
                    Value c;
                    c = a + b;
                    Push(c);
                }
                break;
                case SUB:   // pop 2, push a - b
                {
                    Value b = Pop();
                    Value a = Pop();
                    Value c;
                    c = a - b;
                    Push(c);
                }
                break;
                case MUL:   // pop 2, push a * b
                {
                    Value b = Pop();
                    Value a = Pop();
                    Value c;
                    c = a * b;
                    Push(c);
                }
                break;
                case DIV:   // pop 2, push a / b, push a % b
                {
                    Value b = Pop();
                    Value a = Pop();
                    Value c;
                    Value d;
                    c = a / b;
                    d = fmod(a, b);
                    Push(c);
                    Push(d);
                }
                break;
                case NEG:   // pop 1, push -1
                {
                    Value a = Pop();
                    Value b;
                    b = -a;
                    Push(b);
                }
                break;
                case SHL:    // pop 2, push a << b
                {
                    Value b = Pop();
                    Value a = Pop();
                    uint32_t x = (uint32_t)a;
                    uint32_t y = (uint32_t)b;
                    uint32_t z = x << y;
                    Push((Value)z);
                }
                break;
                case SHR:    // pop 2, push a >> b
                {
                    Value b = Pop();
                    Value a = Pop();
                    uint32_t x = (uint32_t)a;
                    uint32_t y = (uint32_t)b;
                    uint32_t z = x >> y;
                    Push((Value)z);
                }
                break;
                case NOT:    // pop 1, push ~a
                {
                    Value a = Pop();
                    uint32_t x = (uint32_t)a;
                    uint32_t y = ~x;
                    Push((Value)y);
                }
                break;
                case AND:    // pop 2, push a & b
                {
                    Value b = Pop();
                    Value a = Pop();
                    uint32_t x = (uint32_t)a;
                    uint32_t y = (uint32_t)b;
                    uint32_t z = x & y;
                    Push((Value)z);
                }
                break;
                case OR:     // pop 2, push a | b
                {
                    Value b = Pop();
                    Value a = Pop();
                    uint32_t x = (uint32_t)a;
                    uint32_t y = (uint32_t)b;
                    uint32_t z = x | y;
                    Push((Value)z);
                }
                break;
                case XOR:    // pop 2, push a ^ b
                {
                    Value b = Pop();
                    Value a = Pop();
                    uint32_t x = (uint32_t)a;
                    uint32_t y = (uint32_t)b;
                    uint32_t z = x ^ y;
                    Push((Value)z);
                }
                break;
                case NOR:    // pop 2, push ~(a | b)
                {
                    Value b = Pop();
                    Value a = Pop();
                    uint32_t x = (uint32_t)a;
                    uint32_t y = (uint32_t)b;
                    uint32_t z = ~(x | y);
                    Push((Value)z);
                }
                break;
                case NAND:   // pop 2, push ~(a & b)
                {
                    Value b = Pop();
                    Value a = Pop();
                    uint32_t x = (uint32_t)a;
                    uint32_t y = (uint32_t)b;
                    uint32_t z = ~(x & y);
                    Push((Value)z);
                }
                break;
                case JMP:    // pop 1, pc = a
                {
                    Value a = Pop();
                    pc = (int32_t)a;
                }
                break;
                case JZ:    // pop 2, if a == 0: pc = b
                {
                    Value b = Pop();
                    Value a = Pop();
                    if((int32_t)a == 0)
                    {
                        pc = (int32_t)b;
                    }
                }
                break;
                case JNZ:   // pop 2, if a != 0: pc = b
                {
                    Value b = Pop();
                    Value a = Pop();
                    if((int32_t)a != 0)
                    {
                        pc = (int32_t)b;
                    }
                }
                break;
                case JE:    // pop 3, if a == b: pc = c
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    if(a == b)
                    {
                        pc = (int32_t)b;
                    }
                }
                break;
                case JNE:   // pop 3, if a != b pc = c
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    if(a != b)
                    {
                        pc = (int32_t)b;
                    }
                }
                break;
                case JL:    // pop 3, if a < b  pc = c
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    if(a < b)
                    {
                        pc = (int32_t)b;
                    }
                }
                break;
                case JG:    // pop 3, if a > b  pc = c
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    if(a > b)
                    {
                        pc = (int32_t)b;
                    }
                }
                break;
                case JLE:   // pop 3, if a <= b pc = c
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    if(a <= b)
                    {
                        pc = (int32_t)b;
                    }
                }
                break;
                case JGE:   // pop 3, if a >= b pc = c
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    if(a >= b)
                    {
                        pc = (int32_t)b;
                    }
                }
                break;
                case SIN:   // pop 1, push sin(a)
                {
                    Value a = Pop();
                    Value b;
                    b = sin(a);
                    Push(b);
                }
                break;
                case COS:   // pop 1, push cos(a)
                {
                    Value a = Pop();
                    Value b;
                    b = cos(a);
                    Push(b);
                }
                break;
                case TAN:   // pop 1, push tan(a)
                {
                    Value a = Pop();
                    Value b;
                    b = tan(a);
                    Push(b);
                }
                break;
                case ASIN:  // pop 1, push asin(a)
                {
                    Value a = Pop();
                    Value b;
                    b = asin(a);
                    Push(b);
                }
                break;
                case ACOS:  // pop 1, push acos(a)
                {
                    Value a = Pop();
                    Value b;
                    b = acos(a);
                    Push(b);
                }
                break;
                case ATAN:  // pop 2, push atan2(a, b)
                {
                    Value b = Pop();
                    Value a = Pop();
                    Value c;
                    c = atan2(a, b);
                    Push(c);
                }
                break;
                case MIN:   // pop 2, push Min(a, b)
                {
                    Value b = Pop();
                    Value a = Pop();
                    Value c;
                    c = Min(a, b);
                    Push(c);
                }
                break;
                case MAX:   // pop 2, push Max(a, b)
                {
                    Value b = Pop();
                    Value a = Pop();
                    Value c;
                    c = Max(a, b);
                    Push(c);
                }
                break;
                case CLAMP: // pop 3, push Clamp(a, b, c)
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    Value d;
                    d = Clamp(a, b, c);
                    Push(d);
                }
                break;
                case MIX:   // pop 3, push Lerp(a, b, c)
                {
                    Value c = Pop();
                    Value b = Pop();
                    Value a = Pop();
                    Value d;
                    d = Lerp(a, b, c);
                    Push(d);
                }
                break;
                case YIELD:  // pop 1; yield that many times
                {
                    Value a = Pop();
                    m_yield += (int32_t)a;
                    m_pc = pc;
                    return;
                }
                break;
                case SLEEP: // pop 1; sleep for a seconds
                {
                    Value a = Pop();
                    if((int32_t)a > 0)
                    {
                        m_pc = pc;
                        m_sleepBegin = stm_now();
                        m_sleepSecs = (int32_t)a;
                        Sleep();
                    }
                    return;
                }
                break;
                case KILL:   // end execution and free all resources held
                {
                    m_pc = m_codeLen;
                    return;
                }
                break;
            }
        }
    }

    static gen_array<Process> ms_processes;
    
    ProcessHandle Create(const ProcessCreateInfo& info)
    {
        slot s = ms_processes.Create();
        Process& p = ms_processes.GetUnchecked(s);
        p.Init(info);
        return { s };
    }
    void Destroy(ProcessHandle hdl)
    {

    }
    void Wake(ProcessHandle hdl)
    {

    }
    void Sleep(ProcessHandle hdl)
    {

    }
    void Run(ProcessHandle hdl, double usecs)
    {

    }

};  // namespace Script
