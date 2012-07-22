/*
 * Input control file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spInputOutputControl.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#ifdef SP_PLATFORM_WINDOWS
#   include <windows.h>
#   include <mmsystem.h>
#endif

#ifdef SP_PLATFORM_IOS
#   include "Platform/iOS/spPlatformExchangeIOS.h"
#endif


namespace sp
{

extern video::RenderContext* __spRenderContext;

namespace io
{


#if defined(SP_PLATFORM_WINDOWS) || defined(SP_PLATFORM_LINUX)

InputControl::InputControl() : isCursorSpeedBlocked_(false)
{
    LastCursorPos_ = getCursorPosition();
}
InputControl::~InputControl()
{
}

dim::point2di InputControl::getCursorSpeed()
{
    if (!isCursorSpeedBlocked_)
    {
        dim::point2di Pos(getCursorPosition());
        CursorSpeed_            = Pos - LastCursorPos_;
        LastCursorPos_          = Pos;
        isCursorSpeedBlocked_   = true;
    }
    
    return CursorSpeed_;
}

#if defined(SP_PLATFORM_WINDOWS)

void InputControl::setCursorPosition(const dim::point2di &Position, bool UpdateCursorSpeed)
{
    RECT rc;
    GetWindowRect(*(HWND*)__spRenderContext->getWindowObject(), &rc);
    
    SetCursorPos(
        gSharedObjects.ScreenOffsetX + Position.X + rc.left,
        gSharedObjects.ScreenOffsetY + Position.Y + rc.top
    );
    
    if (UpdateCursorSpeed)
        LastCursorPos_ = Position;
}

dim::point2di InputControl::getCursorPosition() const
{
    POINT Position;
    RECT rc;
    
    GetCursorPos(&Position);
    GetWindowRect(*(HWND*)__spRenderContext->getWindowObject(), &rc);
    
    return dim::point2di(
        Position.x - rc.left - gSharedObjects.ScreenOffsetX,
        Position.y - rc.top - gSharedObjects.ScreenOffsetY
    );
}

void InputControl::setCursorVisible(bool Visible)
{
    ShowCursor(Visible);
}

s16 InputControl::getMouseWheel() const
{
    return gSharedObjects.MouseWheel;
}

#elif defined(SP_PLATFORM_LINUX)

void InputControl::setCursorPosition(const dim::point2di &Position, bool UpdateCursorSpeed)
{
    SoftPixelDeviceLinux* DeviceLinux = static_cast<SoftPixelDeviceLinux*>(__spDevice);
    
    XWarpPointer(
        DeviceLinux->Display_, None, DeviceLinux->Window_,
        0, 0,
        DeviceLinux->Resolution_.Width, DeviceLinux->Resolution_.Height,
        Position.X, Position.Y
    );
    
    XFlush(DeviceLinux->Display_);
    
    if (UpdateCursorSpeed)
        LastCursorPos_ = Position;
}

dim::point2di InputControl::getCursorPosition() const
{
    dim::point2di Pos;
    Window TmpWnd;
    s32 TmpPos;
    u32 Mask;
    
    SoftPixelDeviceLinux* DeviceLinux = static_cast<SoftPixelDeviceLinux*>(__spDevice);
    
    XQueryPointer(
        DeviceLinux->Display_, DeviceLinux->Window_, &TmpWnd, &TmpWnd,
        &TmpPos, &TmpPos, &Pos.X, &Pos.Y, &Mask
    );
    
    return Pos;
}

void InputControl::setCursorVisible(bool Visible)
{
    SoftPixelDeviceLinux* DeviceLinux = static_cast<SoftPixelDeviceLinux*>(__spDevice);
    
    if (Visible)
        XUndefineCursor(DeviceLinux->Display_, DeviceLinux->Window_);
    else
        XDefineCursor(DeviceLinux->Display_, DeviceLinux->Window_, DeviceLinux->Cursor_);
}

s16 InputControl::getMouseWheel() const
{
    return gSharedObjects.MouseWheel;
}

#endif

/* === Keyboard === */

bool& InputControl::keyDown(const EKeyCodes KeyCode)
{
    return __isKey[KeyCode];
}
bool& InputControl::keyHit(const EKeyCodes KeyCode)
{
    return __hitKey[KeyCode];
}
bool& InputControl::keyReleased(const EKeyCodes KeyCode)
{
    return __wasKey[KeyCode];
}

/* === Mouse === */

bool& InputControl::mouseDown(const EMouseKeyCodes KeyCode)
{
    return __isMouseKey[KeyCode];
}
bool& InputControl::mouseHit(const EMouseKeyCodes KeyCode)
{
    return __hitMouseKey[KeyCode];
}
bool& InputControl::mouseReleased(const EMouseKeyCodes KeyCode)
{
    return __wasMouseKey[KeyCode];
}

#   if defined(SP_PLATFORM_WINDOWS)

/* === Keyboard === */

void InputControl::keyDownSimulation(const EKeyCodes KeyCode) const
{
    keybd_event((BYTE)KeyCode, (BYTE)KeyCode, 0, GetMessageExtraInfo());
}

void InputControl::keyReleasedSimulation(const EKeyCodes KeyCode) const
{
    keybd_event((BYTE)KeyCode, (BYTE)KeyCode, KEYEVENTF_KEYUP, GetMessageExtraInfo());
}

/* === Mouse === */

void InputControl::mouseDownSimulation(const EMouseKeyCodes KeyCode) const
{
    s32 MouseButton;
    
    switch (KeyCode)
    {
        case MOUSE_LEFT:
            MouseButton = MOUSEEVENTF_LEFTDOWN; break;
        case MOUSE_RIGHT:
            MouseButton = MOUSEEVENTF_RIGHTDOWN; break;
        case MOUSE_MIDDLE:
            MouseButton = MOUSEEVENTF_MIDDLEDOWN; break;
        default:
            return;
    }
    
    dim::point2di Pos(getCursorPosition());
    mouse_event(MouseButton, Pos.X, Pos.Y, 0, GetMessageExtraInfo());
}

void InputControl::mouseReleasedSimulation(const EMouseKeyCodes KeyCode) const
{
    s32 MouseButton;
    
    switch (KeyCode)
    {
        case MOUSE_LEFT:
            MouseButton = MOUSEEVENTF_LEFTUP; break;
        case MOUSE_RIGHT:
            MouseButton = MOUSEEVENTF_RIGHTUP; break;
        case MOUSE_MIDDLE:
            MouseButton = MOUSEEVENTF_MIDDLEUP; break;
        default:
            return;
    }
    
    dim::point2di Pos(getCursorPosition());
    mouse_event(MouseButton, Pos.X, Pos.Y, 0, GetMessageExtraInfo());
}

bool InputControl::keyDownEx(const EKeyCodes KeyCode) const
{
    return HIWORD(GetKeyState((s32)KeyCode)) != 0;
}

void InputControl::setMouseWheel(s16 Value)
{
    dim::point2di Pos(getCursorPosition());
    mouse_event(MOUSEEVENTF_WHEEL, Pos.X, Pos.Y, Value, GetMessageExtraInfo());
}

/* === Joystick === */

dim::vector3df InputControl::getJoystickPosition() const // (-1.0f - +1.0f)
{
    JOYINFO joyInfo;
    
    if (joyGetPos(JOYSTICKID1, &joyInfo) == JOYERR_NOERROR)
    {
        f32 X = (f32)joyInfo.wXpos / __SP_WORDSIZE - 1.0f;
        f32 Y = (f32)joyInfo.wYpos / __SP_WORDSIZE - 1.0f;
        f32 Z = (f32)joyInfo.wZpos / __SP_WORDSIZE;
        return dim::vector3df(X, -Y, Z);
    }
    
    return dim::vector3df();
}

// ! incomplete !
bool InputControl::joystickDown(const EJoystickKeyCodes KeyCode) const
{
    s32 JoystickButton = KeyCode;
    
    JOYINFO joyInfo;
    --JoystickButton;
    
    if (joyGetPos(JOYSTICKID1, &joyInfo) == JOYERR_NOERROR)
    {
        s32 Count = (s32)pow(2.0f, JoystickButton);
        for (s32 i = 0; i < Count; i += (s32)pow(2.0f, i))
        {
            if (joyInfo.wButtons == Count)
                return true;
            joyInfo.wButtons -= i;
        }
    }
    
    return false;
}

#   elif defined(SP_PLATFORM_LINUX)

bool InputControl::keyDownEx(const EKeyCodes KeyCode) const
{
    return __isKey[KeyCode];
}

dim::vector3df InputControl::getJoystickPosition() const // (-1.0f - +1.0f)
{
    return dim::vector3df(); // !TODO
}

bool InputControl::joystickDown(const EJoystickKeyCodes KeyCode) const
{
    return false; // !TODO
}

#   endif

#elif defined(SP_PLATFORM_ANDROID)

InputControl::InputControl()
{
    /* Initialization */
    isCursorSpeedBlocked_       = false;
    isMotionEvent_              = false;
    isDefaultKeyEventHandling_  = true;
    
    memset(isKeyEnabled_, 1, sizeof(bool) * (KEY_BUTTON_MODE + 1));
    
    PointerCount_               = 0;
    
    DragDistance_               = 0.0f;
    LastDragDistance_           = 0.0f;
    LightSensor_                = 0.0f;
}
InputControl::~InputControl()
{
}

s32 InputControl::processEvent(android_app* App, AInputEvent* Event)
{
    const int32_t Type = AInputEvent_getType(Event);
    
    switch (Type)
    {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            PointerCount_   = math::Min<s32>(AMotionEvent_getPointerCount(Event), 5);
            isMotionEvent_  = true;
            
            for (s32 i = 0; i < PointerCount_; ++i)
            {
                setPointer(
                    i, dim::point2df(AMotionEvent_getX(Event, i), AMotionEvent_getY(Event, i)), AMotionEvent_getSize(Event, i)
                );
            }
        }
        return 1;
        
        case AINPUT_EVENT_TYPE_KEY:
        {
            const u8 KeyCode = static_cast<u8>(AKeyEvent_getKeyCode(Event));
            
            switch (AKeyEvent_getAction(Event))
            {
                case AKEY_EVENT_ACTION_DOWN:
                    if (!__isKey[KeyCode])
                        __hitKey[KeyCode] = true;
                    __isKey[KeyCode] = true;
                    break;
                case AKEY_EVENT_ACTION_UP:
                    __wasKey[KeyCode] = true;
                    __isKey[KeyCode] = false;
                    break;
            }
            
            if (!isDefaultKeyEventHandling_)
                return 1;
            return isKeyEnabled_[KeyCode] ? 0 : 1;
        }
        break;
        
        default:
            break;
    }
    
    return 0;
}

dim::point2df InputControl::getPointerPosition(u32 Index) const
{
    return (Index < POINTER_COUNT) ? Pointer_[Index].Position : 0.0f;
}
f32 InputControl::getPointerSize(u32 Index) const
{
    return (Index < POINTER_COUNT) ? Pointer_[Index].Size : 0.0f;
}

dim::point2df InputControl::getMotionSpeed(u32 Index)
{
    if (Index < POINTER_COUNT)
    {
        if (!isCursorSpeedBlocked_)
        {
            MotionSpeed_[Index]     = MotionPointerPos_[Index] - LastPointerPos_[Index];
            LastPointerPos_[Index]  = MotionPointerPos_[Index];
            isCursorSpeedBlocked_   = true;
        }
        return MotionSpeed_[Index];
    }
    return 0.0f;
}

bool InputControl::isPointerActive(u32 Index) const
{
    return Index < PointerCount_;
}

f32 InputControl::getPinchSpeed()
{
    if (PointerCount_ >= 2)
    {
        const f32 CurDistance = math::getDistance(Pointer_[0].Position, Pointer_[1].Position);
        
        if (DragDistance_ < -0.5f)
            LastDragDistance_ = CurDistance;
        else
            LastDragDistance_ = DragDistance_;
        
        DragDistance_ = CurDistance;
        
        return DragDistance_ - LastDragDistance_;
    }
    return 0.0f;
}

bool& InputControl::keyDown(const EKeyCodes KeyCode)
{
    return __isKey[KeyCode];
}
bool& InputControl::keyHit(const EKeyCodes KeyCode)
{
    return __hitKey[KeyCode];
}
bool& InputControl::keyReleased(const EKeyCodes KeyCode)
{
    return __wasKey[KeyCode];
}


/*
 * ======= Private: =======
 */

void InputControl::setPointer(u32 Index, const dim::point2df &Position, f32 Size)
{
    if (Index < PointerCount_)
    {
        /* Update motion speed */
        if (LastPointerPos_[Index].X < -0.5f || Position.X < -0.5f)
            LastPointerPos_[Index] = Position;
        MotionPointerPos_[Index] = Position;
        
        /* Set pointer position */
        if (Position.X >= -0.5f)
        {
            Pointer_[Index].Position    = Position;
            Pointer_[Index].Size        = Size;
        }
    }
}

void InputControl::resetInputEvents()
{
    for (s32 i = 0; i < POINTER_COUNT; ++i)
        setPointer(i, -1.0f, 0.0f);
    DragDistance_ = -1.0f;
}

#elif defined(SP_PLATFORM_IOS)

InputControl::InputControl()
{
    memset(iOS_TouchEvents, 0, sizeof(SiOS_TouchEvent) * 5);
    
    DragDistance_       = 0.0f;
    LastDragDistance_   = 0.0f;
}
InputControl::~InputControl()
{
}

dim::point2di InputControl::getPointerPosition(u32 Index) const
{
    if (Index < 5 && iOS_TouchEvents[Index].Active)
        return dim::point2di(iOS_TouchEvents[Index].X, iOS_TouchEvents[Index].Y);
    return 0;
}

dim::point2di InputControl::getMotionSpeed(u32 Index) const
{
    if (Index < 5 && iOS_TouchEvents[Index].Active)
    {
        if (iOS_TouchEvents[Index].Reset)
        {
            iOS_TouchEvents[Index].SpeedX = iOS_TouchEvents[Index].X - iOS_TouchEvents[Index].LastX;
            iOS_TouchEvents[Index].SpeedY = iOS_TouchEvents[Index].Y - iOS_TouchEvents[Index].LastY;
            
            iOS_TouchEvents[Index].LastX = iOS_TouchEvents[Index].X;
            iOS_TouchEvents[Index].LastY = iOS_TouchEvents[Index].Y;
            
            iOS_TouchEvents[Index].Reset = 0;
        }
        
        return dim::point2di(iOS_TouchEvents[Index].SpeedX, iOS_TouchEvents[Index].SpeedY);
    }
    
    return 0;
}

bool InputControl::isPointerActive(u32 Index) const
{
    return Index < 5 ? iOS_TouchEvents[Index].Active : false;
}

f32 InputControl::getPinchSpeed()
{
    if (iOS_TouchEventCount >= 2)
    {
        const f32 CurDistance = math::getDistance(
            dim::point2df(iOS_TouchEvents[0].X, iOS_TouchEvents[0].Y),
            dim::point2df(iOS_TouchEvents[1].X, iOS_TouchEvents[1].Y)
        );
        
        if (DragDistance_ < -0.5f)
            LastDragDistance_ = CurDistance;
        else
            LastDragDistance_ = DragDistance_;
        
        DragDistance_ = CurDistance;
        
        return DragDistance_ - LastDragDistance_;
    }
    return 0.0f;
}

u32 InputControl::getPointerCount() const
{
    return iOS_TouchEventCount;
}


/*
 * ======= Private: =======
 */

void InputControl::resetInputEvents()
{
    iOS_TouchEventCount = 0;
    
    for (s32 i = 0; i < 5; ++i)
    {
        iOS_TouchEvents[i].Reset = 1;
        if (iOS_TouchEvents[i].Active)
            ++iOS_TouchEventCount;
    }
    
    DragDistance_ = -1.0f;
}

#endif


} // /namespace io

} // /namespace sp



// ================================================================================
