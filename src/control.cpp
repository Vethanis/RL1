#include "control.h"
#include "array.h"
#include "fnv.h"
#include "window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Control
{
    struct ActionBinding
    {
        ButtonLogic m_logic;
        State*      m_cur;
    };
    struct AxisBinding
    {
        AxisLogic   m_logic;
        float*     m_cur;
    };

    typedef FixedArray<ActionBinding, 16> ActionBindings;
    typedef FixedArray<AxisBinding, 8> AxisBindings;

    static Array<ActionBindings> ms_actionBindings;
    static Array<uint64_t>      ms_actionHashes;

    static Array<AxisBindings>  ms_axesBindings;
    static Array<uint64_t>      ms_axesHashes;

    static float                ms_dt;
    static bool                 ms_cursorHidden = true;
    static bool                 ms_skipCursorUpdate = false;

    static float                ms_mouseAxis[MA_COUNT * 2];
    static float                ms_gameAxis[GPA_COUNT * 2];

    static State                ms_keys[K_COUNT * 2];
    static State                ms_mouseButtons[MB_COUNT * 2];
    static State                ms_gameButtons[GPB_COUNT * 2];

    void Init()
    {
        ms_dt = 1.0f / 60.0f;
        for(float& x : ms_mouseAxis)
        {
            x = 0.0f;
        }
        for(float& x : ms_gameAxis)
        {
            x = 0.0f;
        }
        MemZero(ms_keys);
        MemZero(ms_mouseButtons);
        MemZero(ms_gameButtons);
        ms_actionBindings.clear();
        ms_actionHashes.clear();
        ms_axesBindings.clear();
        ms_axesHashes.clear();
    }
    void Update(float dt)
    {
        ms_dt = dt;

        // copy current state to previous state
        for(int32_t i = 0; i < MA_COUNT; ++i)
        {
            ms_mouseAxis[i * 2 + 1] = ms_mouseAxis[i * 2];
        }
        for(int32_t i = K_FIRST; i < K_COUNT; ++i)
        {
            ms_keys[i * 2 + 1] = ms_keys[i * 2];
        }
        for(int32_t i = MB_1; i < MB_COUNT; ++i)
        {
            ms_mouseButtons[i * 2 + 1] = ms_mouseButtons[i * 2];
        }
        for(int32_t i = 0; i < GPA_COUNT; ++i)
        {
            ms_gameAxis[i * 2 + 1] = ms_gameAxis[i * 2];
        }
        for(int32_t i = 0; i < GPB_COUNT; ++i)
        {
            ms_gameButtons[i * 2 + 1] = ms_gameButtons[i * 2];
        }

        // update current state
        glfwPollEvents();

        {
            GLFWwindow* window = Window::GetActive()->m_window;
            int32_t width, height;
            glfwGetWindowSize(window, &width, &height);
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            x /= (double)width;
            y /= (double)height;
            x = x * 2.0 - 1.0;
            y = y * 2.0 - 1.0;

            ms_mouseAxis[MA_Cursor_X * 2] = (float)-x;
            ms_mouseAxis[MA_Cursor_Y * 2] = (float)-y;

            if(ms_skipCursorUpdate)
            {
                ms_skipCursorUpdate = false;
                ms_mouseAxis[MA_Cursor_X * 2 + 1] = ms_mouseAxis[MA_Cursor_X * 2];
                ms_mouseAxis[MA_Cursor_Y * 2 + 1] = ms_mouseAxis[MA_Cursor_Y * 2];
            }
        }

        {
            GLFWgamepadstate state;
            MemZero(state);
            glfwGetGamepadState(0, &state);
            for(int32_t i = 0; i < GPA_COUNT; ++i)
            {
                ms_gameAxis[i * 2] = state.axes[i];
            }
            for(int32_t i = 0; i < GPB_COUNT; ++i)
            {
                ms_gameButtons[i * 2] = state.buttons[i] ? Press : Release;
            }
        }
    }
    void Shutdown()
    {
        ms_actionBindings.reset();
        ms_actionHashes.reset();
        ms_axesBindings.reset();
        ms_axesHashes.reset();
    }

    void SetCursorHidden(bool hidden)
    {
        ms_cursorHidden = hidden;            
        ms_skipCursorUpdate = true;
        GLFWwindow* window = Window::GetActive()->m_window;
        if(ms_cursorHidden)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    bool IsCursorHidden()
    {
        return ms_cursorHidden;
    }

    void CloseMainWindow()
    {
        GLFWwindow* window = Window::GetActive()->m_window;
        glfwSetWindowShouldClose(window, true);
    }

    uint8_t EvalLogic(uint8_t cur, uint8_t next, ButtonLogic logic)
    {            
        switch(logic)
        {
            case BL_OR:
                cur = cur | next;
            break;
            case BL_AND:
                cur = cur & next;
            break;
            case BL_XOR:
                cur = cur ^ next;
            break;
            case BL_NOR:
                cur = ~(cur | next);
            case BL_NAND:
                cur = ~(cur & next);
            break;
        }
        cur = cur & 1u;
        return cur;
    }
    float EvalLogic(float cur, float next, AxisLogic logic)
    {
        switch(logic)
        {
            case AL_MIN:
                cur = Min(cur, next);
            break;
            case AL_MAX:
                cur = Max(cur, next);
            break;
            case AL_SMALLEST:
                if(fabs(cur) < fabs(next))
                {
                    cur = cur;
                }
                else
                {
                    cur = next;
                }
            break;
            case AL_LARGEST:
                if(fabs(cur) > fabs(next))
                {
                    cur = next;
                }
                else
                {
                    cur = cur;
                }
            break;
        }
        return cur;
    }
    void RemoveFromBinding(ActionBindings& bindings, State* ptr)
    {
        for(int32_t i = 0; i < bindings.count(); ++i)
        {
            if(bindings[i].m_cur == ptr)
            {
                bindings.shiftRemove(i);
                --i;
            }
        }
    }
    void RemoveFromBinding(AxisBindings& bindings, float* ptr)
    {
        for(int32_t i = 0; i < bindings.count(); ++i)
        {
            if(bindings[i].m_cur == ptr)
            {
                bindings.shiftRemove(i);
                --i;
            }
        }
    }

    int32_t RegisterAction(const char* name)
    {
        uint64_t hash = Fnv64(name);
        int32_t idx = ms_actionHashes.find(hash);
        if(idx != -1)
        {
            return idx;
        }
        idx = ms_actionHashes.count();
        ms_actionHashes.grow() = hash;
        MemZero(ms_actionBindings.grow());
        return idx;
    }
    int32_t GetActionLocation(const char* name)
    {
        return ms_actionHashes.find(Fnv64(name));
    }
    void BindToAction(int32_t location, ButtonLogic logic, Key key)
    {
        Assert(location != -1);
        ActionBindings& bindings = ms_actionBindings[location];
        if(!bindings.full())
        {
            ActionBinding& binding = bindings.grow();
            binding.m_logic = logic;
            binding.m_cur = ms_keys + key * 2;
        }
    }
    void BindToAction(int32_t location, ButtonLogic logic, MouseButton button)
    {
        Assert(location != -1);
        ActionBindings& bindings = ms_actionBindings[location];
        if(!bindings.full())
        {
            ActionBinding& binding = bindings.grow();
            binding.m_logic = logic;
            binding.m_cur = ms_mouseButtons + button * 2;
        }
    }
    void BindToAction(int32_t location, ButtonLogic logic, GamePadButton button)
    {
        Assert(location != -1);
        ActionBindings& bindings = ms_actionBindings[location];
        if(!bindings.full())
        {
            ActionBinding& binding = bindings.grow();
            binding.m_logic = logic;
            binding.m_cur = ms_gameButtons + button * 2;
        }
    }
    void UnbindFromAction(int32_t location, Key key)
    {
        Assert(location != -1);
        ActionBindings& bindings = ms_actionBindings[location];
        State* ptr = ms_keys + key * 2;
        RemoveFromBinding(bindings, ptr);
    }
    void UnbindFromAction(int32_t location, MouseButton button)
    {
        Assert(location != -1);
        ActionBindings& bindings = ms_actionBindings[location];
        State* ptr = ms_mouseButtons + button * 2;
        RemoveFromBinding(bindings, ptr);
    }
    void UnbindFromAction(int32_t location, GamePadButton button)
    {
        Assert(location != -1);
        ActionBindings& bindings = ms_actionBindings[location];
        State* ptr = ms_gameButtons + button * 2;
        RemoveFromBinding(bindings, ptr);
    }
    State GetActionState(int32_t location)
    {
        Assert(location != -1);
        const ActionBindings& bindings = ms_actionBindings[location];
        if(bindings.empty())
        {
            return Release;
        }
        uint8_t state = *(bindings[0].m_cur);
        for(int32_t i = 1; i < bindings.count(); ++i)
        {
            uint8_t nextState = *(bindings[i].m_cur);
            state = EvalLogic(state, nextState, bindings[i].m_logic);
        }
        return (State)state;
    }
    State GetPrevActionState(int32_t location)
    {
        Assert(location != -1);
        const ActionBindings& bindings = ms_actionBindings[location];
        if(bindings.empty())
        {
            return Release;
        }
        uint8_t state = *(bindings[0].m_cur + 1);
        for(int32_t i = 1; i < bindings.count(); ++i)
        {
            uint8_t nextState = *(bindings[i].m_cur + 1);
            state = EvalLogic(state, nextState, bindings[i].m_logic);
        }
        return (State)state;
    }
    Transition GetActionTransition(int32_t location)
    {
        State cur = GetActionState(location);
        State prev = GetPrevActionState(location);
        if(cur == prev)
        {
            return NoChange;
        }
        return cur ? ReleaseToPress : PressToRelease;
    }

    int32_t RegisterAxis(const char* name)
    {
        uint64_t hash = Fnv64(name);
        int32_t idx = ms_axesHashes.find(hash);
        if(idx != -1)
        {
            return idx;
        }
        idx = ms_axesHashes.count();
        ms_axesHashes.grow() = hash;
        MemZero(ms_axesBindings.grow());
        return idx;
    }
    int32_t GetAxisLocation(const char* name)
    {
        return ms_axesHashes.find(Fnv64(name));
    }
    void BindToAxis(int32_t location, AxisLogic logic, MouseAxis axis)
    {
        Assert(location != -1);
        AxisBindings& bindings = ms_axesBindings[location];
        if(!bindings.full())
        {
            AxisBinding& binding = bindings.grow();
            binding.m_logic = logic;
            binding.m_cur = ms_mouseAxis + axis * 2;
        }
    }
    void BindToAxis(int32_t location, AxisLogic logic, GamePadAxis axis)
    {
        Assert(location != -1);
        AxisBindings& bindings = ms_axesBindings[location];
        if(!bindings.full())
        {
            AxisBinding& binding = bindings.grow();
            binding.m_logic = logic;
            binding.m_cur = ms_gameAxis + axis * 2;
        }
    }
    void UnbindFromAxis(int32_t location, MouseAxis axis)
    {
        Assert(location != -1);
        AxisBindings& bindings = ms_axesBindings[location];
        float* ptr = ms_mouseAxis + axis * 2;
        RemoveFromBinding(bindings, ptr);
    }
    void UnbindFromAxis(int32_t location, GamePadAxis axis)
    {
        Assert(location != -1);
        AxisBindings& bindings = ms_axesBindings[location];
        float* ptr = ms_gameAxis + axis * 2;
        RemoveFromBinding(bindings, ptr);
    }
    float GetAxisControl(int32_t location)
    {
        Assert(location != -1);
        const AxisBindings& bindings = ms_axesBindings[location];
        if(bindings.empty())
        {
            return 0.0f;
        }
        float value = *(bindings[0].m_cur);
        for(int32_t i = 1; i < bindings.count(); ++i)
        {
            value = EvalLogic(value, *(bindings[i].m_cur), bindings[i].m_logic);
        }
        return value;
    }
    float GetPrevAxisControl(int32_t location)
    {
        Assert(location != -1);
        const AxisBindings& bindings = ms_axesBindings[location];
        if(bindings.empty())
        {
            return 0.0f;
        }
        float value = *(bindings[0].m_cur + 1);
        for(int32_t i = 1; i < bindings.count(); ++i)
        {
            value = EvalLogic(value, *(bindings[i].m_cur + 1), bindings[i].m_logic);
        }
        return value;
    }
    float GetAxisControlDelta(int32_t location)
    {
        return GetAxisControl(location) - GetPrevAxisControl(location);
    }

    void KeyCB(int32_t key, int32_t action, int32_t mods)
    {
        ms_keys[key * 2] = action != GLFW_RELEASE ? Press : Release;
    }
    void MouseButtonCB(int32_t button, int32_t action, int32_t mods)
    {
        ms_mouseButtons[button * 2] = action != GLFW_RELEASE ? Press : Release;
    }
    void ScrollWheelCB(float dx, float dy)
    {
        ms_mouseAxis[MA_Scroll_X * 2] += dx;
        ms_mouseAxis[MA_Scroll_Y * 2] += dy;
    }
};
