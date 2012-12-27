/*
 * XBox 360 gamepad file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spXBox360GamePad.hpp"

#ifdef SP_COMPILE_WITH_XBOX360GAMEPAD


#include "Base/spMathCore.hpp"

#include <windows.h>
#include <XInput.h>


namespace sp
{
namespace io
{


static const s32 XBOX_JOYSTICK_MIN = 32768;
static const s32 XBOX_JOYSTICK_MAX = 32767;

static XINPUT_STATE XBoxCtrlStates_[MAX_XBOX_CONTROLLERS];
static XINPUT_VIBRATION XBoxCtrlVirbations_[MAX_XBOX_CONTROLLERS];

XBox360GamePad::XBox360GamePad(s32 Number) :
    Number_     (math::MinMax(Number, 0, 3) ),
    Connected_  (false                      )
{
}
XBox360GamePad::~XBox360GamePad()
{
}

bool XBox360GamePad::buttonHit(const EGamePadButtons Button) const
{
    return false; //!TODO!
}
bool XBox360GamePad::buttonDown(const EGamePadButtons Button) const
{
    return (XBoxCtrlStates_[Number_].Gamepad.wButtons & Button) != 0;
}
bool XBox360GamePad::buttonReleased(const EGamePadButtons Button) const
{
    return false; //!TODO!
}

u8 XBox360GamePad::getLeftTrigger() const
{
    return XBoxCtrlStates_[Number_].Gamepad.bLeftTrigger;
}
u8 XBox360GamePad::getRightTrigger() const
{
    return XBoxCtrlStates_[Number_].Gamepad.bRightTrigger;
}

static inline dim::point2df convertJoystickAxes(s32 X, s32 Y)
{
    return dim::point2df(
        X < 0 ? static_cast<f32>(X) / XBOX_JOYSTICK_MIN : static_cast<f32>(X) / XBOX_JOYSTICK_MAX,
        Y < 0 ? static_cast<f32>(Y) / XBOX_JOYSTICK_MIN : static_cast<f32>(Y) / XBOX_JOYSTICK_MAX
    );
}

dim::point2df XBox360GamePad::getLeftJoystick() const
{
    return convertJoystickAxes(
        XBoxCtrlStates_[Number_].Gamepad.sThumbLX,
        XBoxCtrlStates_[Number_].Gamepad.sThumbLY
    );
}
dim::point2df XBox360GamePad::getRightJoystick() const
{
    return convertJoystickAxes(
        XBoxCtrlStates_[Number_].Gamepad.sThumbRX,
        XBoxCtrlStates_[Number_].Gamepad.sThumbRY
    );
}

dim::point2di XBox360GamePad::getLeftJoystickNative() const
{
    return dim::point2di(
        XBoxCtrlStates_[Number_].Gamepad.sThumbLX,
        XBoxCtrlStates_[Number_].Gamepad.sThumbLY
    );
}
dim::point2di XBox360GamePad::getRightJoystickNative() const
{
    return dim::point2di(
        XBoxCtrlStates_[Number_].Gamepad.sThumbRX,
        XBoxCtrlStates_[Number_].Gamepad.sThumbRY
    );
}

void XBox360GamePad::setVibration(u16 MotorSpeed)
{
    setVibration(MotorSpeed, MotorSpeed);
}
void XBox360GamePad::setVibration(u16 LeftMotorSpeed, u16 RightMotorSpeed)
{
    XBoxCtrlVirbations_[Number_].wLeftMotorSpeed = LeftMotorSpeed;
    XBoxCtrlVirbations_[Number_].wRightMotorSpeed = RightMotorSpeed;
    XInputSetState(Number_, &XBoxCtrlVirbations_[Number_]);
}
void XBox360GamePad::setLeftVibration(u16 MotorSpeed)
{
    XBoxCtrlVirbations_[Number_].wLeftMotorSpeed = MotorSpeed;
    XInputSetState(Number_, &XBoxCtrlVirbations_[Number_]);
}
void XBox360GamePad::setRightVibration(u16 MotorSpeed)
{
    XBoxCtrlVirbations_[Number_].wRightMotorSpeed = MotorSpeed;
    XInputSetState(Number_, &XBoxCtrlVirbations_[Number_]);
}


/*
 * ======= Private: =======
 */

void XBox360GamePad::updateState()
{
    /* Update controller state */
    DWORD Result = XInputGetState(Number_, &XBoxCtrlStates_[Number_]);
    
    /* Update connection state */
    bool Connected = Connected_;
    
    if (Result == ERROR_SUCCESS)
        Connected = true;
    else
        Connected = false;
    
    if (Connected_ != Connected)
    {
        Connected_ = Connected;
        if (ConnectCallback_)
            ConnectCallback_(*this);
    }
}


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
