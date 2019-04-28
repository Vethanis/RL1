#pragma once

#include "enum.h"

#define CHANNEL_ENUM(X) \
    X(Channel_Null, = 0) \
    X(Window_Width,) \
    X(Window_Height,) \
    X(Key_Space, = 32) \
    X(Key_Apostrophe, = 39) \
    X(Key_Comma, = 44) \
    X(Key_Minus, = 45) \
    X(Key_Period, = 46) \
    X(Key_Slash, = 47) \
    X(Key_0, = 48) \
    X(Key_1, = 49) \
    X(Key_2, = 50) \
    X(Key_3, = 51) \
    X(Key_4, = 52) \
    X(Key_5, = 53) \
    X(Key_6, = 54) \
    X(Key_7, = 55) \
    X(Key_8, = 56) \
    X(Key_9, = 57) \
    X(Key_Semicolon, = 59) \
    X(Key_Equal, = 61) \
    X(Key_A, = 65) \
    X(Key_B, = 66) \
    X(Key_C, = 67) \
    X(Key_D, = 68) \
    X(Key_E, = 69) \
    X(Key_F, = 70) \
    X(Key_G, = 71) \
    X(Key_H, = 72) \
    X(Key_I, = 73) \
    X(Key_J, = 74) \
    X(Key_K, = 75) \
    X(Key_L, = 76) \
    X(Key_M, = 77) \
    X(Key_N, = 78) \
    X(Key_O, = 79) \
    X(Key_P, = 80) \
    X(Key_Q, = 81) \
    X(Key_R, = 82) \
    X(Key_S, = 83) \
    X(Key_T, = 84) \
    X(Key_U, = 85) \
    X(Key_V, = 86) \
    X(Key_W, = 87) \
    X(Key_X, = 88) \
    X(Key_Y, = 89) \
    X(Key_Z, = 90) \
    X(Key_LeftBracket, = 91) \
    X(Key_Backslash, = 92) \
    X(Key_RightBracket, = 93) \
    X(Key_GraveAccent, = 96) \
    X(Key_Escape, = 256) \
    X(Key_Enter, = 257) \
    X(Key_Tab, = 258) \
    X(Key_Backspace, = 259) \
    X(Key_Insert, = 260) \
    X(Key_Delete, = 261) \
    X(Key_Right, = 262) \
    X(Key_Left, = 263) \
    X(Key_Down, = 264) \
    X(Key_Up, = 265) \
    X(Key_PageUp, = 266) \
    X(Key_PageDown, = 267) \
    X(Key_Home, = 268) \
    X(Key_End, = 269) \
    X(Key_CapsLock, = 280) \
    X(Key_ScrollLock, = 281) \
    X(Key_NumLock, = 282) \
    X(Key_PrintScreen, = 283) \
    X(Key_Pause, = 284) \
    X(Key_F1, = 290) \
    X(Key_F2, = 291) \
    X(Key_F3, = 292) \
    X(Key_F4, = 293) \
    X(Key_F5, = 294) \
    X(Key_F6, = 295) \
    X(Key_F7, = 296) \
    X(Key_F8, = 297) \
    X(Key_F9, = 298) \
    X(Key_F10, = 299) \
    X(Key_F11, = 300) \
    X(Key_F12, = 301) \
    X(Key_F13, = 302) \
    X(Key_F14, = 303) \
    X(Key_F15, = 304) \
    X(Key_F16, = 305) \
    X(Key_F17, = 306) \
    X(Key_F18, = 307) \
    X(Key_F19, = 308) \
    X(Key_F20, = 309) \
    X(Key_F21, = 310) \
    X(Key_F22, = 311) \
    X(Key_F23, = 312) \
    X(Key_F24, = 313) \
    X(Key_F25, = 314) \
    X(Key_KP_0, = 320) \
    X(Key_KP_1, = 321) \
    X(Key_KP_2, = 322) \
    X(Key_KP_3, = 323) \
    X(Key_KP_4, = 324) \
    X(Key_KP_5, = 325) \
    X(Key_KP_6, = 326) \
    X(Key_KP_7, = 327) \
    X(Key_KP_8, = 328) \
    X(Key_KP_9, = 329) \
    X(Key_KP_Decimal, = 330) \
    X(Key_KP_Divide, = 331) \
    X(Key_KP_Multiply, = 332) \
    X(Key_KP_Subtract, = 333) \
    X(Key_KP_Add, = 334) \
    X(Key_KP_Enter, = 335) \
    X(Key_KP_Equal, = 336) \
    X(Key_LeftShift, = 340) \
    X(Key_LeftControl, = 341) \
    X(Key_LeftAlt, = 342) \
    X(Key_LeftSuper, = 343) \
    X(Key_RightShift, = 344) \
    X(Key_RightControl, = 345) \
    X(Key_RightAlt, = 346) \
    X(Key_RightSuper, = 347) \
    X(Key_Menu, = 348) \
    X(MouseAxis_X,) \
    X(MouseAxis_Y,) \
    X(MouseAxis_Z,) \
    X(PadAxis_LX,) \
    X(PadAxis_LY,) \
    X(PadAxis_RX,) \
    X(PadAxis_RY,) \
    X(PadAxis_LTrig,) \
    X(PadAxis_RTrig,) \
    X(MouseBtn_1,) \
    X(MouseBtn_2,) \
    X(MouseBtn_3,) \
    X(MouseBtn_4,) \
    X(MouseBtn_5,) \
    X(MouseBtn_6,) \
    X(MouseBtn_7,) \
    X(MouseBtn_8,) \
    X(PadBtn_A,) \
    X(PadBtn_B,) \
    X(PadBtn_X,) \
    X(PadBtn_Y,) \
    X(PadBtn_LBump,) \
    X(PadBtn_RBump,) \
    X(PadBtn_Back,) \
    X(PadBtn_Start,) \
    X(PadBtn_Guide,) \
    X(PadBtn_LThumb,) \
    X(PadBtn_RThumb,) \
    X(PadBtn_Up,) \
    X(PadBtn_Right,) \
    X(PadBtn_Down,) \
    X(PadBtn_Left,) \
    X(Channel_Count,)

ENUM_DECL(Channel, CHANNEL_ENUM)
