/*
 * XBox 360 gamepad file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spXBox360GamePad.hpp"

#ifdef SP_COMPILE_WITH_XBOX360GAMEPAD


#include "Base/spMathCore.hpp"
#include "Base/spTimer.hpp"

#include <windows.h>
#include <XInput.h>


namespace sp
{
namespace io
{


//! Keycode chart from 'DirectX SDK Docu'
static const s32 XBOX_BUTTON_KEYCODES[GAMEPAD_BUTTON_COUNT] =
{
    0x00000001,
    0x00000002,
    0x00000004,
    0x00000008,
    0x00000010,
    0x00000020,
    0x00000040,
    0x00000080,
    0x0100,
    0x0200,
    0x1000,
    0x2000,
    0x4000,
    0x8000,
};

static XINPUT_STATE XBoxCtrlStates_[MAX_XBOX_CONTROLLERS];
static XINPUT_VIBRATION XBoxCtrlVirbations_[MAX_XBOX_CONTROLLERS];

XBox360GamePad::XBox360GamePad(s32 Number) :
    Number_     (math::MinMax(Number, 0, 3) ),
    Connected_  (false                      )
{
    clearButtonStates();
}
XBox360GamePad::~XBox360GamePad()
{
}

bool XBox360GamePad::buttonHit(const EGamePadButtons Button) const
{
    return Connected_ && Button >= 0 && Button < GAMEPAD_BUTTON_COUNT && ButtonHitSet_[Button];
}
bool XBox360GamePad::buttonDown(const EGamePadButtons Button) const
{
    return Connected_ && Button >= 0 && Button < GAMEPAD_BUTTON_COUNT &&
        ((XBoxCtrlStates_[Number_].Gamepad.wButtons & XBOX_BUTTON_KEYCODES[Button]) != 0);
}
bool XBox360GamePad::buttonReleased(const EGamePadButtons Button) const
{
    return Connected_ && Button >= 0 && Button < GAMEPAD_BUTTON_COUNT && ButtonReleasedSet_[Button];
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

dim::point2df XBox360GamePad::getLeftJoystick(f32 Threshold) const
{
    dim::point2df Dir(convertJoystickAxes(
        XBoxCtrlStates_[Number_].Gamepad.sThumbLX,
        XBoxCtrlStates_[Number_].Gamepad.sThumbLY
    ));
    
    if (math::Abs(Dir.X) < Threshold)
        Dir.X = 0.0f;
    if (math::Abs(Dir.Y) < Threshold)
        Dir.Y = 0.0f;
    
    return Dir;
}
dim::point2df XBox360GamePad::getRightJoystick(f32 Threshold) const
{
    dim::point2df Dir(convertJoystickAxes(
        XBoxCtrlStates_[Number_].Gamepad.sThumbRX,
        XBoxCtrlStates_[Number_].Gamepad.sThumbRY
    ));
    
    if (math::Abs(Dir.X) < Threshold)
        Dir.X = 0.0f;
    if (math::Abs(Dir.Y) < Threshold)
        Dir.Y = 0.0f;
    
    return Dir;
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

void XBox360GamePad::setLeftVibration(u16 MotorSpeed, u64 Duration)
{
    setLeftVibration(MotorSpeed);
    VibrationDuration_[0] = (Duration > 0 ? io::Timer::millisecs() + Duration : 0);
}
void XBox360GamePad::setRightVibration(u16 MotorSpeed, u64 Duration)
{
    setRightVibration(MotorSpeed);
    VibrationDuration_[1] = (Duration > 0 ? io::Timer::millisecs() + Duration : 0);
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
        clearButtonStates();
        if (ConnectCallback_)
            ConnectCallback_(*this);
    }
    
    /* Update extended events */
    if (Connected_)
        updateButtonStates();
}

void XBox360GamePad::clearButtonStates()
{
    memset(VibrationDuration_, 0, sizeof(VibrationDuration_));
    memset(ButtonHitSet_, 0, sizeof(ButtonHitSet_));
    memset(ButtonWasHitSet_, 0, sizeof(ButtonWasHitSet_));
    memset(ButtonReleasedSet_, 0, sizeof(ButtonReleasedSet_));
}

//!TODO! -> reset states on window lost focus
void XBox360GamePad::updateButtonStates()
{
    /* Update vibration timer */
    if (VibrationDuration_[0] > 0 && io::Timer::millisecs() > VibrationDuration_[0])
    {
        VibrationDuration_[0] = 0;
        setLeftVibration(0);
    }
    if (VibrationDuration_[1] > 0 && io::Timer::millisecs() > VibrationDuration_[1])
    {
        VibrationDuration_[1] = 0;
        setRightVibration(0);
    }
    
    /* Update gamepad button states */
    for (s32 i = 0; i < GAMEPAD_BUTTON_COUNT; ++i)
    {
        if (buttonDown(static_cast<EGamePadButtons>(i)))
        {
            if (!ButtonWasHitSet_[i])
            {
                ButtonWasHitSet_[i] = true;
                ButtonHitSet_[i] = true;
            }
            else
                ButtonHitSet_[i] = false;
        }
        else
        {
            if (ButtonWasHitSet_[i])
            {
                ButtonWasHitSet_[i] = false;
                ButtonReleasedSet_[i] = true;
            }
            else
                ButtonReleasedSet_[i] = false;
        }
    }
}


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
