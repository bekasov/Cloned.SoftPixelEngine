/*
 * XBox 360 gamepad header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_XBOX360_GAMEPAD_H__
#define __SP_XBOX360_GAMEPAD_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_XBOX360GAMEPAD


#include "Base/spInputOutputString.hpp"
#include "Base/spDimensionVector2D.hpp"
#include "Base/spInputKeyCodes.hpp"

#include <boost/function.hpp>


namespace sp
{
namespace io
{


static const u32 MAX_XBOX_CONTROLLERS = 4;

class XBox360GamePad;

//! XBox 360 gamepad shared pointer type.
typedef boost::shared_ptr<XBox360GamePad> XBox360GamePadPtr;

//! XBox 360 gamepad connection callback type.
typedef boost::function<void (const XBox360GamePad &Controller)> XBoxGamePadConnectCallback;


static const u16 MAX_GAMEPAD_VIRBATION  = 65535;
static const s32 XBOX_JOYSTICK_MIN      = 32768;
static const s32 XBOX_JOYSTICK_MAX      = 32767;


/**
The XBox360 game pad class was designed particular for the "XBox Gamepad" on MS/Windows.
\since Version 3.2
*/
class SP_EXPORT XBox360GamePad
{
    
    public:
        
        //! \param[in] Number Specifies the controller number. Must be 0, 1, 2 or 3.
        XBox360GamePad(s32 Number);
        ~XBox360GamePad();
        
        /* === Functions === */
        
        //! Returns true if the specified gamepad button was hit.
        bool buttonHit(const EGamePadButtons Button) const;
        //! Returns true if the specified gamepad button is pressed.
        bool buttonDown(const EGamePadButtons Button) const;
        //! Returns true if the specified gamepad button was released.
        bool buttonReleased(const EGamePadButtons Button) const;
        
        //! Returns the left trigger value. This value is in the range [0 .. 255].
        u8 getLeftTrigger() const;
        //! Returns the right trigger value. This value is in the range [0 .. 255].
        u8 getRightTrigger() const;
        
        /**
        Returns the left joystick coordinates. The coordinates are in the range [0.0 .. 1.0].
        Positive values signify up or right.
        \param[in] Threshold Specifies the threshold factor to eliminate imprecision.
        Should by in the range [0.0 .. 1.0]. By default 0.1.
        */
        dim::point2df getLeftJoystick(f32 Threshold = 0.15f) const;
        /**
        Returns the right joystick coordinates.
        \see getLeftJoystick
        */
        dim::point2df getRightJoystick(f32 Threshold = 0.15f) const;
        
        //! Returns the native left joystick values. The values are in the range [-XBOX_JOYSTICK_MIN .. XBOX_JOYSTICK_MAX]
        dim::point2di getLeftJoystickNative() const;
        //! Returns the native right joystick values. The values are in the range [-XBOX_JOYSTICK_MIN .. XBOX_JOYSTICK_MAX]
        dim::point2di getRightJoystickNative() const;
        
        //! Sets the left and right vibration motor speed. Use MAX_GAMEPAD_VIRBATION for maximal vibration speed.
        void setVibration(u16 MotorSpeed);
        //! Sets the left and right vibration motor speed. Use MAX_GAMEPAD_VIRBATION for maximal vibration speed.
        void setVibration(u16 LeftMotorSpeed, u16 RightMotorSpeed);
        
        /**
        Sets the left vibration motor speed. Use MAX_GAMEPAD_VIRBATION for maximal vibration speed.
        \note The left vibration feels different, than the right! The left one is rough.
        */
        void setLeftVibration(u16 MotorSpeed);
        /**
        Sets the right vibration motor speed. Use MAX_GAMEPAD_VIRBATION for maximal vibration speed.
        \note The right vibration feels different, than the left! The right one is fine.
        */
        void setRightVibration(u16 MotorSpeed);
        
        /**
        Sets the left vibration motor speed for the specified duration.
        \param[in] MotorSpeed Specifies the motor speed. Use MAX_GAMEPAD_VIRBATION for maximal vibration speed.
        \param[in] Duration Specifies the vibration duration (in milliseconds). Use 0 to deactivate the timer.
        */
        void setLeftVibration(u16 MotorSpeed, u64 Duration);
        /**
        Sets the right vibration motor speed for the specified duration.
        \see setLeftVibration
        */
        void setRightVibration(u16 MotorSpeed, u64 Duration);
        
        /* === Inline functions === */
        
        /**
        Returns true if this controller is connected.
        \see setConnectCallback
        */
        inline bool connected() const
        {
            return Connected_;
        }
        //! Returns the controller number. Must be 0, 1, 2 or 3.
        inline s32 getNumber() const
        {
            return Number_;
        }
        
        //! Sets the connection callback. Use this to determine when a gamepad is connected or disconnected.
        inline void setConnectCallback(const XBoxGamePadConnectCallback &Callback)
        {
            ConnectCallback_ = Callback;
        }
        
    private:
        
        friend class InputControl;
        
        /* === Functions === */
        
        void updateState();
        void clearButtonStates();
        void updateButtonStates();
        
        /* === Members === */
        
        s32 Number_;
        bool Connected_;
        
        XBoxGamePadConnectCallback ConnectCallback_;
        
        u64 VibrationDuration_[2];
        
        bool ButtonHitSet_[GAMEPAD_BUTTON_COUNT];
        bool ButtonWasHitSet_[GAMEPAD_BUTTON_COUNT];
        bool ButtonReleasedSet_[GAMEPAD_BUTTON_COUNT];
        
};


} // /namespace io

} // /namespace sp


#endif

#endif



// ================================================================================
