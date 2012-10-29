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

//! User input control interface for PC operating-systems (Windows, Linux etc.). Keyboard, mouse and joystick input examination.
class SP_EXPORT InputControl
{
    
    public:
        
        InputControl();
        ~InputControl();
        
        /* === Functions === */
        
        //! Puts the cursor at the specified position.
        void setCursorPosition(const dim::point2di &Position, bool UpdateCursorSpeed = true);
        dim::point2di getCursorPosition() const;
        
        //! Retunrs cursor motion speed.
        dim::point2di getCursorSpeed();
        
        /**
        Returns true if the specified key is pressed otherwise False.
        \return Reference to the stored key state. This can be modified for your own purposes.
        */
        bool& keyDown(const EKeyCodes KeyCode);
        //! \see keyDown
        bool& keyHit(const EKeyCodes KeyCode);
        //! \see keyDown
        bool& keyReleased(const EKeyCodes KeyCode);
        
        //! Simulates a key press (down or release). This effects the whole system!
        void keyDownSimulation(const EKeyCodes KeyCode) const;
        void keyReleasedSimulation(const EKeyCodes KeyCode) const;
        
        /**
        Returns true if the specified mouse button is pressed otherwise false.
        \return Reference to the stored mouse button state. This can be modified for your own purposes.
        */
        bool& mouseDown(const EMouseKeyCodes KeyCode);
        //! \see mouseDown
        bool& mouseHit(const EMouseKeyCodes KeyCode);
        //! \see mouseDown
        bool& mouseReleased(const EMouseKeyCodes KeyCode);
        //! \see mouseDown
        bool& mouseDoubleClicked(const EMouseKeyCodes KeyCode);
        
        void mouseDownSimulation(const EMouseKeyCodes KeyCode) const;
        void mouseReleasedSimulation(const EMouseKeyCodes KeyCode) const;
        
        //! Returns the mouse wheel motion.
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
        
        /* === Inline functions === */
        
        /**
        Enables the word (or rather character) input. When word input is enabled,
        you'll receive a string the user can write like in a text box.
        By default disabled.
        \see getEnteredWord
        */
        inline void setWordInput(bool Enable)
        {
            WordInput_ = Enable;
        }
        //! Returns true if word input is enabled. By default disabled.
        inline bool getWordInput() const
        {
            return WordInput_;
        }
        
        /**
        Returns the entered word string.
        \see setWordInput
        */
        inline io::stringc getEnteredWord() const
        {
            return WordString_;
        }
        /**
        Returns a reference to the entered word string.
        Use this to reset the enterd word.
        \see setWordInput
        */
        inline io::stringc& getEnteredWord()
        {
            return WordString_;
        }
        /**
        Stores the entered word string into the given parameter and releases the clears string.
        \see setWordInput.
        */
        inline void releaseEnteredWord(io::stringc &Str)
        {
            Str = WordString_;
            WordString_.clear();
        }
        
        /* === Static functions === */
        
        //! Clears the keyboard input events.
        static void clearInput();
        //! Resets the keyboard input events. This is called every time "SoftPixelDevice::updateEvent" is called.
        static void resetInput();
        
        //! Records a key down events. Use this if you have your own window callback.
        static void keyEventDown(u32 KeyCode);
        //! Records a key up events. Use this if you have your own window callback.
        static void keyEventUp(u32 KeyCode);
        
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
        
        static void recordKey(u32 KeyCode);
        
        /* Members */
        
        bool isCursorSpeedBlocked_;
        dim::point2di LastCursorPos_, CursorSpeed_;
        
        bool WordInput_;
        io::stringc WordString_;
        
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
