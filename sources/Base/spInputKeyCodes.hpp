/*
 * Input key codes header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_KEYCODES_H__
#define __SP_INPUTOUTPUT_KEYCODES_H__


#include "Base/spStandard.hpp"


namespace sp
{
namespace io
{


#if defined(SP_PLATFORM_WINDOWS) || defined(SP_PLATFORM_LINUX)

//! Types of input devices.
enum EInputTypes
{
    INPUTTYPE_KEYBOARD      = 0x01, //!< EKeyCodes is used.
    INPUTTYPE_MOUSE         = 0x02, //!< EMouseKeyCodes is used.
    INPUTTYPE_MOUSEWHEEL    = 0x04, //!< EMouseWheelMotions is used.
    INPUTTYPE_JOYSTICK      = 0x08, //!< EJoystickKeyCodes is used.
    
    INPUTTYPE_ANY           = INPUTTYPE_KEYBOARD | INPUTTYPE_MOUSE | INPUTTYPE_MOUSEWHEEL | INPUTTYPE_JOYSTICK,
};

//! Mouse key code enumeration.
enum EMouseKeyCodes
{
    MOUSE_LEFT = 0, //!< Left mouse button.
    MOUSE_MIDDLE,   //!< Middle mouse button. Actually the mouse wheel.
    MOUSE_RIGHT,    //!< Right mouse button.
};

//! Mouse wheel motion enumeration.
enum EMouseWheelMotions
{
    MOUSEWHEEL_NONE = 0,    //!< No mouse wheel motion.
    MOUSEWHEEL_UP   = 1,    //!< Mouse wheel up motion.
    MOUSEWHEEL_DOWN = -1,   //!< Mouse wheel down motion.
};

//! Joystick button key code enumeration.
enum EJoystickKeyCodes
{
    JOYSTICK_KEY1 = 1,
    JOYSTICK_KEY2,
    JOYSTICK_KEY3,
    JOYSTICK_KEY4,
    JOYSTICK_KEY5,
    JOYSTICK_KEY6,
    JOYSTICK_KEY7,
    JOYSTICK_KEY8,
    JOYSTICK_KEY9,
    JOYSTICK_KEY10,
    JOYSTICK_KEY11,
    JOYSTICK_KEY12,
    JOYSTICK_KEY13,
    JOYSTICK_KEY14,
    JOYSTICK_KEY15,
    JOYSTICK_KEY16,
};

//! Key code enumeration.
enum EKeyCodes
{
    KEY_LBUTTON     = 0x01, //!< Left mouse button
    KEY_RBUTTON     = 0x02, //!< Right mouse button
    KEY_CANCEL      = 0x03, //!< Control-break processing
    KEY_MBUTTON     = 0x04, //!< Middle mouse button (three-button mouse)
    KEY_XBUTTON1    = 0x05, //!< Windows 2000/XP: X1 mouse button
    KEY_XBUTTON2    = 0x06, //!< Windows 2000/XP: X2 mouse button
    KEY_BACK        = 0x08, //!< BACKSPACE key
    KEY_TAB         = 0x09, //!< TAB key
    KEY_CLEAR       = 0x0C, //!< CLEAR key
    KEY_RETURN      = 0x0D, //!< ENTER key
    KEY_SHIFT       = 0x10, //!< SHIFT key
    KEY_CONTROL     = 0x11, //!< CTRL key
    KEY_MENU        = 0x12, //!< ALT key
    KEY_PAUSE       = 0x13, //!< PAUSE key
    KEY_CAPITAL     = 0x14, //!< CAPS LOCK key
    KEY_KANA        = 0x15, //!< IME Kana mode
    KEY_HANGUEL     = 0x15, //!< IME Hanguel mode
    KEY_HANGUL      = 0x15, //!< IME Hangul mode
    KEY_JUNJA       = 0x17, //!< IME Junja mode
    KEY_FINAL       = 0x18, //!< IME final mode
    KEY_HANJA       = 0x19, //!< IME Hanja mode
    KEY_KANJI       = 0x19, //!< IME Kanji mode
    
    KEY_ESCAPE      = 0x1B,
    KEY_SPACE       = 0x20,
    KEY_PAGEUP      = 0x21,
    KEY_PAGEDOWN    = 0x22,
    KEY_END         = 0x23,
    KEY_HOME        = 0x24,
    KEY_LEFT        = 0x25, //!< Left arrow key
    KEY_UP          = 0x26, //!< Up arrow key
    KEY_RIGHT       = 0x27, //!< Right arrow key
    KEY_DOWN        = 0x28, //!< Down arrow key
    KEY_SELECT      = 0x29,
    KEY_EXE         = 0x2B, //!< Execute key
    KEY_SNAPSHOT    = 0x2C,
    KEY_INSERT      = 0x2D,
    KEY_DELETE      = 0x2E,
    KEY_HELP        = 0x2F,
    
    KEY_0           = 0x30,
    KEY_1           = 0x31,
    KEY_2           = 0x32,
    KEY_3           = 0x33,
    KEY_4           = 0x34,
    KEY_5           = 0x35,
    KEY_6           = 0x36,
    KEY_7           = 0x37,
    KEY_8           = 0x38,
    KEY_9           = 0x39,
    
    KEY_A           = 0x41,
    KEY_B           = 0x42,
    KEY_C           = 0x43,
    KEY_D           = 0x44,
    KEY_E           = 0x45,
    KEY_F           = 0x46,
    KEY_G           = 0x47,
    KEY_H           = 0x48,
    KEY_I           = 0x49,
    KEY_J           = 0x4A,
    KEY_K           = 0x4B,
    KEY_L           = 0x4C,
    KEY_M           = 0x4D,
    KEY_N           = 0x4E,
    KEY_O           = 0x4F,
    KEY_P           = 0x50,
    KEY_Q           = 0x51,
    KEY_R           = 0x52,
    KEY_S           = 0x53,
    KEY_T           = 0x54,
    KEY_U           = 0x55,
    KEY_V           = 0x56,
    KEY_W           = 0x57,
    KEY_X           = 0x58,
    KEY_Y           = 0x59,
    KEY_Z           = 0x5A,
    
    KEY_WINLEFT     = 0x5B,
    KEY_WINRIGHT    = 0x5C,
    KEY_APPS        = 0x5D,
    
    KEY_NUMPAD0     = 0x60,
    KEY_NUMPAD1     = 0x61,
    KEY_NUMPAD2     = 0x62,
    KEY_NUMPAD3     = 0x63,
    KEY_NUMPAD4     = 0x64,
    KEY_NUMPAD5     = 0x65,
    KEY_NUMPAD6     = 0x66,
    KEY_NUMPAD7     = 0x67,
    KEY_NUMPAD8     = 0x68,
    KEY_NUMPAD9     = 0x69,
    
    KEY_MULTIPLY    = 0x6A,
    KEY_ADD         = 0x6B,
    KEY_SEPARATOR   = 0x6C,
    KEY_SUBTRACT    = 0x6D,
    KEY_DECIMAL     = 0x6E,
    KEY_DIVIDE      = 0x6F,
    
    KEY_F1          = 0x70,
    KEY_F2          = 0x71,
    KEY_F3          = 0x72,
    KEY_F4          = 0x73,
    KEY_F5          = 0x74,
    KEY_F6          = 0x75,
    KEY_F7          = 0x76,
    KEY_F8          = 0x77,
    KEY_F9          = 0x78,
    KEY_F10         = 0x79,
    KEY_F11         = 0x7A,
    KEY_F12         = 0x7B,
    KEY_F13         = 0x7C,
    KEY_F14         = 0x7D,
    KEY_F15         = 0x7E,
    KEY_F16         = 0x7F,
    KEY_F17         = 0x80,
    KEY_F18         = 0x81,
    KEY_F19         = 0x82,
    KEY_F20         = 0x83,
    KEY_F21         = 0x84,
    KEY_F22         = 0x85,
    KEY_F23         = 0x86,
    KEY_F24         = 0x87,
    
    KEY_NUMLOCK     = 0x90,
    KEY_SCROLL      = 0x91,
    
    KEY_LSHIFT      = 0xA0,
    KEY_RSHIFT      = 0xA1,
    KEY_LCONTROL    = 0xA2,
    KEY_RCONTROL    = 0xA3,
    KEY_LMENU       = 0xA4,
    KEY_RMENU       = 0xA5,
    
    KEY_PLUS        = 0xBB, //!< '+'
    KEY_COMMA       = 0xBC, //!< ','
    KEY_MINUS       = 0xBD, //!< '-'
    KEY_PERIOD      = 0xBE, //!< '.'
    
    KEY_EXPONENT    = 0xDC, //!< '^'
    
    KEY_ANY         = 0xF0, //!< This is not a special key. It is used to determine that any key is pressed, hit or release.
    
    KEY_ATTN        = 0xF6,
    KEY_CRSEL       = 0xF7,
    KEY_EXSEL       = 0xF8,
    KEY_EREOF       = 0xF9,
    KEY_PLAY        = 0xFA,
    KEY_ZOOM        = 0xFB,
    KEY_NONAME      = 0xFC,
    KEY_PA1         = 0xFD,
    KEY_OEMCLEAR    = 0xFE,
};

#elif defined(SP_PLATFORM_ANDROID)

enum EKeyCodes
{
    KEY_UNKNOWN = 0,
    KEY_SOFT_LEFT,
    KEY_SOFT_RIGHT,
    KEY_HOME,
    KEY_BACK,
    KEY_CALL,
    KEY_ENDCALL,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_STAR,
    KEY_POUND,
    KEY_DPAD_UP,
    KEY_DPAD_DOWN,
    KEY_DPAD_LEFT,
    KEY_DPAD_RIGHT,
    KEY_DPAD_CENTER,
    KEY_VOLUME_UP,
    KEY_VOLUME_DOWN,
    KEY_POWER,
    KEY_CAMERA,
    KEY_CLEAR,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_ALT_LEFT,
    KEY_ALT_RIGHT,
    KEY_SHIFT_LEFT,
    KEY_SHIFT_RIGHT,
    KEY_TAB,
    KEY_SPACE,
    KEY_SYM,
    KEY_EXPLORER,
    KEY_ENVELOPE,
    KEY_ENTER,
    KEY_DEL,
    KEY_GRAVE,
    KEY_MINUS,
    KEY_EQUALS,
    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,
    KEY_BACKSLASH,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_SLASH,
    KEY_AT,
    KEY_NUM,
    KEY_HEADSETHOOK,
    KEY_FOCUS,
    KEY_PLUS,
    KEY_MENU,
    KEY_NOTIFICATION,
    KEY_SEARCH,
    KEY_MEDIA_PLAY_PAUSE,
    KEY_MEDIA_STOP,
    KEY_MEDIA_NEXT,
    KEY_MEDIA_PREVIOUS,
    KEY_MEDIA_REWIND,
    KEY_MEDIA_FAST_FORWARD,
    KEY_MUTE,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_PICTSYMBOLS,
    KEY_SWITCH_CHARSET,
    KEY_BUTTON_A,
    KEY_BUTTON_B,
    KEY_BUTTON_C,
    KEY_BUTTON_X,
    KEY_BUTTON_Y,
    KEY_BUTTON_Z,
    KEY_BUTTON_L1,
    KEY_BUTTON_R1,
    KEY_BUTTON_L2,
    KEY_BUTTON_R2,
    KEY_BUTTON_THUMBL,
    KEY_BUTTON_THUMBR,
    KEY_BUTTON_START,
    KEY_BUTTON_SELECT,
    KEY_BUTTON_MODE,
};

#endif


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
