#pragma once

#include <stdint.h>

namespace Control
{
    enum ButtonLogic : uint8_t
    {
        BL_OR = 0,
        BL_AND,
        BL_XOR,
        BL_NOR,
        BL_NAND,
        BL_COUNT
    };

    enum AxisLogic : uint8_t
    {
        AL_LARGEST = 0,
        AL_SMALLEST,
        AL_MAX,
        AL_MIN,
        AL_COUNT
    };

    enum MouseAxis : uint8_t
    {
        MA_Cursor_X = 0,
        MA_Cursor_Y,
        MA_Scroll_X,
        MA_Scroll_Y,
        MA_COUNT
    };

    enum GamePadAxis : uint8_t
    {
        GPA_LEFT_X = 0,
        GPA_LEFT_Y,
        GPA_RIGHT_X,
        GPA_RIGHT_Y,
        GPA_LEFT_TRIGGER,
        GPA_RIGHT_TRIGGER,
        GPA_COUNT
    };

    enum MouseButton : uint8_t
    {
        MB_1 = 0,
        MB_2,
        MB_3,
        MB_4,
        MB_5,
        MB_6,
        MB_7,
        MB_8,
        MB_COUNT
    };

    enum GamePadButton : uint8_t
    {
        GPB_A = 0,
        GPB_B,
        GPB_X,
        GPB_Y,
        GPB_LEFT_BUMPER,
        GPB_RIGHT_BUMPER,
        GPB_BACK,
        GPB_START,
        GPB_GUIDE,
        GPB_LEFT_THUMB,
        GPB_RIGHT_THUMB,
        GPB_DPAD_UP,
        GPB_DPAD_RIGHT,
        GPB_DPAD_DOWN,
        GPB_DPAD_LEFT,
        GPB_COUNT
    };

    enum Key : uint16_t
    {
        K_FIRST = 32,
        K_SPACE = 32,
        K_APOSTROPHE = 39,
        K_COMMA = 44,
        K_MINUS = 45,
        K_PERIOD = 46,
        K_SLASH = 47,
        K_0 = 48,
        K_1 = 49,
        K_2 = 50,
        K_3 = 51,
        K_4 = 52,
        K_5 = 53,
        K_6 = 54,
        K_7 = 55,
        K_8 = 56,
        K_9 = 57,
        K_SEMICOLON = 59,
        K_EQUAL = 61,
        K_A = 65,
        K_B = 66,
        K_C = 67,
        K_D = 68,
        K_E = 69,
        K_F = 70,
        K_G = 71,
        K_H = 72,
        K_I = 73,
        K_J = 74,
        K_K = 75,
        K_L = 76,
        K_M = 77,
        K_N = 78,
        K_O = 79,
        K_P = 80,
        K_Q = 81,
        K_R = 82,
        K_S = 83,
        K_T = 84,
        K_U = 85,
        K_V = 86,
        K_W = 87,
        K_X = 88,
        K_Y = 89,
        K_Z = 90,
        K_LEFT_BRACKET = 91,
        K_BACKSLASH = 92,
        K_RIGHT_BRACKET = 93,
        K_GRAVE_ACCENT = 96,
        K_ESCAPE = 256,
        K_ENTER = 257,
        K_TAB = 258,
        K_BACKSPACE = 259,
        K_INSERT = 260,
        K_DELETE = 261,
        K_RIGHT = 262,
        K_LEFT = 263,
        K_DOWN = 264,
        K_UP = 265,
        K_PAGE_UP = 266,
        K_PAGE_DOWN = 267,
        K_HOME = 268,
        K_END = 269,
        K_CAPS_LOCK = 280,
        K_SCROLL_LOCK = 281,
        K_NUM_LOCK = 282,
        K_PRINT_SCREEN = 283,
        K_PAUSE = 284,
        K_F1 = 290,
        K_F2 = 291,
        K_F3 = 292,
        K_F4 = 293,
        K_F5 = 294,
        K_F6 = 295,
        K_F7 = 296,
        K_F8 = 297,
        K_F9 = 298,
        K_F10 = 299,
        K_F11 = 300,
        K_F12 = 301,
        K_F13 = 302,
        K_F14 = 303,
        K_F15 = 304,
        K_F16 = 305,
        K_F17 = 306,
        K_F18 = 307,
        K_F19 = 308,
        K_F20 = 309,
        K_F21 = 310,
        K_F22 = 311,
        K_F23 = 312,
        K_F24 = 313,
        K_F25 = 314,
        K_KP_0 = 320,
        K_KP_1 = 321,
        K_KP_2 = 322,
        K_KP_3 = 323,
        K_KP_4 = 324,
        K_KP_5 = 325,
        K_KP_6 = 326,
        K_KP_7 = 327,
        K_KP_8 = 328,
        K_KP_9 = 329,
        K_KP_DECIMAL = 330,
        K_KP_DIVIDE = 331,
        K_KP_MULTIPLY = 332,
        K_KP_SUBTRACT = 333,
        K_KP_ADD = 334,
        K_KP_ENTER = 335,
        K_KP_EQUAL = 336,
        K_LEFT_SHIFT = 340,
        K_LEFT_CONTROL = 341,
        K_LEFT_ALT = 342,
        K_LEFT_SUPER = 343,
        K_RIGHT_SHIFT = 344,
        K_RIGHT_CONTROL = 345,
        K_RIGHT_ALT = 346,
        K_RIGHT_SUPER = 347,
        K_MENU = 348,
        K_COUNT = 349
    };

    enum State : uint8_t
    {
        Release = 0,
        Press = 1,
    };

    enum Transition : uint8_t
    {
        NoChange = 0,
        ReleaseToPress,
        PressToRelease,
    };

    void Init();
    void Update(float dt);
    void Shutdown();

    void SetCursorHidden(bool hidden);
    bool IsCursorHidden();

    void CloseMainWindow();

    int32_t RegisterAction(const char* name);
    int32_t GetActionLocation(const char* name);
    void BindToAction(int32_t location, ButtonLogic logic, Key key);
    void BindToAction(int32_t location, ButtonLogic logic, MouseButton button);
    void BindToAction(int32_t location, ButtonLogic logic, GamePadButton button);
    void UnbindFromAction(int32_t location, Key key);
    void UnbindFromAction(int32_t location, MouseButton button);
    void UnbindFromAction(int32_t location, GamePadButton button);
    State GetActionState(int32_t location);
    State GetPrevActionState(int32_t location);
    Transition GetActionTransition(int32_t location);

    int32_t RegisterAxis(const char* name);
    int32_t GetAxisLocation(const char* name);
    void BindToAxis(int32_t location, AxisLogic logic, MouseAxis axis);
    void BindToAxis(int32_t location, AxisLogic logic, GamePadAxis axis);
    void UnbindFromAxis(int32_t location, MouseAxis axis);
    void UnbindFromAxis(int32_t location, GamePadAxis axis);
    float GetAxisControl(int32_t location);
    float GetPrevAxisControl(int32_t location);
    float GetAxisControlDelta(int32_t location);

    void KeyCB(int32_t key, int32_t action, int32_t mods);
    void MouseButtonCB(int32_t button, int32_t action, int32_t mods);
    void ScrollWheelCB(float dx, float dy);
};
