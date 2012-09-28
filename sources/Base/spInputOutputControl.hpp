/*
 * Input control header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_CONTROL_H__
#define __SP_INPUTOUTPUT_CONTROL_H__


#include "Base/spStandard.hpp"
#include "Base/spInputKeyCodes.hpp"
#include "Base/spDimensionPoint2D.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spInputOutputLog.hpp"

#include <math.h>

#if defined(SP_PLATFORM_LINUX)
#   include <X11/X.h>
#   include <X11/Xlib.h>
#   include <X11/Xutil.h>
#elif defined(SP_PLATFORM_ANDROID)
#   include <android/input.h>
#   include "Platform/Android/android_native_app_glue.h"
#endif


namespace sp
{

class SoftPixelDevice;

#if defined(SP_PLATFORM_LINUX)
class SoftPixelDeviceLinux;
#elif defined(SP_PLATFORM_ANDROID)
class SoftPixelDeviceAndroid;
#elif defined(SP_PLATFORM_IOS)
class SoftPixelDeviceIOS;
#endif

#ifdef SP_COMPILE_WITH_OPENGL
namespace video
{
    class RenderContext;
}
#endif

namespace io
{


#if defined(SP_PLATFORM_WINDOWS) || defined(SP_PLATFORM_LINUX)

/*
 * Macros
 */

#define __SP_WORDSIZE           32767.5f // (65536 / 2)
#define __SP_IGNORE_CURSORPOS   10000


//! User input control interface for PC operating-systems (Windows, Linux etc.). Keyboard, mouse and joystick input examination.
class SP_EXPORT InputControl
{
    
    public:
        
        InputControl();
        ~InputControl();
        
        //! Clears the keyboard input events.
        static void clearInput();
        
        //! Puts the cursor at the specified position.
        void setCursorPosition(const dim::point2di &Position, bool UpdateCursorSpeed = true);
        dim::point2di getCursorPosition() const;
        
        //! Retunrs cursor motion speed.
        dim::point2di getCursorSpeed();
        
        /**
        \return True if the specified key is pressed otherwise False. The return value is a reference, thus you
        can manipulate this value if you want the further keyboard-checks will or will not be effected
        after pressing this key.
        */
        bool& keyDown(const EKeyCodes KeyCode);     // Key pressed
        bool& keyHit(const EKeyCodes KeyCode);      // Key hit
        bool& keyReleased(const EKeyCodes KeyCode); // Key released
        
        //! Simulates a key press (down or release). This effects the whole system!
        void keyDownSimulation(const EKeyCodes KeyCode) const;
        void keyReleasedSimulation(const EKeyCodes KeyCode) const;
        
        /**
        Similiar to "keyDown" but supports extended keys like "Left Control"/ "Right Control" or
        "Left Shift"/ "Right Shift" etc.
        */
        bool keyDownEx(const EKeyCodes KeyCode) const;  // Extendet key pressed method (e.g. for LShift/ RShift)
        
        //! \return True if the specified mouse button is pressed otherwise False.
        bool& mouseDown(const EMouseKeyCodes KeyCode);      // Mouse button pressed
        bool& mouseHit(const EMouseKeyCodes KeyCode);       // Mouse button hit
        bool& mouseReleased(const EMouseKeyCodes KeyCode);  // Mouse button released
        
        void mouseDownSimulation(const EMouseKeyCodes KeyCode) const;
        void mouseReleasedSimulation(const EMouseKeyCodes KeyCode) const;
        
        //! \return Mouse wheel motion speed.
        s16 getMouseWheel() const;
        
        //! Simulates a mouse wheel motion.
        void setMouseWheel(s16 Value);
        
        /**
        Enables or disables the cursor visibility. In window mode the cursor is by default visible and in fullscreen
        mode by default invisible.
        */
        void setCursorVisible(bool Visible);
        
        //! Returns true if the specified joystick button is pressed.
        bool joystickDown(const EJoystickKeyCodes KeyCode) const;
        
        //! Returns 3D vector of the joystick position (X and Y coordinates are most important).
        dim::vector3df getJoystickPosition() const;
        
    private:
        
        friend class sp::SoftPixelDevice;
        friend class video::RenderContext;
        
        #if defined(SP_PLATFORM_LINUX)
        friend class sp::SoftPixelDeviceLinux;
        #endif
        
        /* Functions */
        
        /**
        Updates the previous cursor position. This is used internally to update
        cursor speed correctly while switching between several render contexts.
        */
        void updatePrevCursorPosition(const dim::point2di &PositionShift);
        
        /* Members */
        
        bool isCursorSpeedBlocked_;
        dim::point2di LastCursorPos_, CursorSpeed_;
        
};

#elif defined(SP_PLATFORM_ANDROID)

//! User input control interface for Mobile operating-systems (Android).
class SP_EXPORT InputControl
{
    
    public:
        
        InputControl();
        ~InputControl();
        
        /* === Functions === */
        
        s32 processEvent(android_app* App, AInputEvent* Event);
        
        //! Returns the position of the specified pointer in screen space.
        dim::point2df getPointerPosition(u32 Index = 0) const;
        
        //! Returns the pressure size (in the range [0.0, 1.0]) of the specified pointer.
        f32 getPointerSize(u32 Index = 0) const;
        
        dim::point2df getMotionSpeed(u32 Index = 0);
        
        //! Returns true if the specified pointer is active (i.e. touch events are performed).
        bool isPointerActive(u32 Index = 0) const;
        
        //! Returns the pinch speed. This is the speed when you pinch with two fingers. Thus the first two pointers are used.
        f32 getPinchSpeed();
        
        /**
        \return True if the specified key is pressed otherwise false. The return value is a reference, thus you
        can manipulate this value if you want the further keyboard-checks will or will not be effected
        after pressing this key.
        */
        bool& keyDown(const EKeyCodes KeyCode);
        bool& keyHit(const EKeyCodes KeyCode);
        bool& keyReleased(const EKeyCodes KeyCode);
        
        /* === Inline functions === */
        
        inline u32 getPointerCount() const
        {
            return PointerCount_;
        }
        
        inline bool isMotionEvent() const
        {
            return isMotionEvent_;
        }
        
        inline dim::vector3df getGyroscopeSensor() const
        {
            return GyroscopeSensor_;
        }
        inline dim::vector3df getAccelerometerSensor() const
        {
            return AccelerometerSensor_;
        }
        inline f32 getLightSensor() const
        {
            return LightSensor_;
        }
        
        /**
        Enables or disables the default key event handling. If disabled keys such as "volume up/down" won't occur
        a higher or lower sound volume. By default enabled.
        */
        inline void setDefaultKeyEventHandling(bool Enable)
        {
            isDefaultKeyEventHandling_ = Enable;
        }
        inline bool getDefaultKeyEventHandling() const
        {
            return isDefaultKeyEventHandling_;
        }
        
        //! Enables or disables each individual key. This has only an effect if default-key-event-handling is disabled.
        inline void setKeyEnable(const EKeyCodes KeyCode, bool Enable)
        {
            isKeyEnabled_[KeyCode] = Enable;
        }
        inline bool getKeyEnable(const EKeyCodes KeyCode) const
        {
            return isKeyEnabled_[KeyCode];
        }
        
    private:
        
        friend class sp::SoftPixelDevice;
        friend class sp::SoftPixelDeviceAndroid;
        
        /* Macros */
        
        static const s32 POINTER_COUNT = 5;
        
        /* Structures */
        
        struct STouchPointer
        {
            dim::point2df Position;
            f32 Size;
        };
        
        /* Functions */
        
        void setPointer(u32 Index, const dim::point2df &Position, f32 Size);
        void resetInputEvents();
        
        /* Members */
        
        bool isCursorSpeedBlocked_;
        bool isMotionEvent_;
        
        bool isDefaultKeyEventHandling_;
        bool isKeyEnabled_[KEY_BUTTON_MODE + 1];
        
        u32 PointerCount_;
        STouchPointer Pointer_[POINTER_COUNT];
        dim::point2df MotionSpeed_[POINTER_COUNT], MotionPointerPos_[POINTER_COUNT], LastPointerPos_[POINTER_COUNT];
        f32 DragDistance_, LastDragDistance_;
        
        dim::vector3df GyroscopeSensor_, AccelerometerSensor_;
        f32 LightSensor_;
        
};

#elif defined(SP_PLATFORM_IOS)

//! User input control interface for Apple iOS.
class SP_EXPORT InputControl
{
    
    public:
        
        InputControl();
        ~InputControl();
        
        /* === Functions === */
        
        //! Returns the position of the specified pointer in screen space.
        dim::point2di getPointerPosition(u32 Index = 0) const;
        
        //! Returns the motion speed of the specified pointer in screen space.
        dim::point2di getMotionSpeed(u32 Index = 0) const;
        
        //! Returns true if the specified pointer is active (i.e. touch events are performed).
        bool isPointerActive(u32 Index = 0) const;
        
        //! Returns the pinch speed. This is the speed when you pinch with two fingers. Thus the first two pointers are used.
        f32 getPinchSpeed();
        
        //! Returns the number of pointers.
        u32 getPointerCount() const;
        
    private:
        
        friend class sp::SoftPixelDevice;
        friend class sp::SoftPixelDeviceIOS;
        
        /* Macros */
        
        static const s32 POINTER_COUNT = 5;
        
        /* Functions */
        
        void resetInputEvents();
        
        /* Members */
        
        bool isCursorSpeedBlocked_;
        
        f32 DragDistance_, LastDragDistance_;
        
};

#endif


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
