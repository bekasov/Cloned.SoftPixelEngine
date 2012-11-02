/*
 * Input service header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_INPUTSERVICE_H__
#define __SP_UTILITY_INPUTSERVICE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_INPUTSERVICE


#include "Base/spInputOutputControl.hpp"

#include <map>


namespace sp
{
namespace tool
{


/**
The input service is used to simplify the input event handling for your games.
Here is a small example which shows how to use the InputService class:
\code
enum MyGameInputEvents
{
    GAME_INPUT_MOVE_FORWARDS,
    GAME_INPUT_MOVE_BACKWARDS,
    GAME_INPUT_MOVE_LEFT,
    GAME_INPUT_MOVE_RIGHT,
    GAME_INPUT_JUMP,
    GAME_INPUT_SHOOT,
    GAME_INPUT_RELOAD,
    GAME_INPUT_WEAPON_NEXT,
    GAME_INPUT_WEAPON_PREVIOUS,
};

// ...

// Pass the io::InputControl device you are using.
InputService service(spControl);

service.addEvent(GAME_INPUT_MOVE_FORWARDS,      io::KEY_W           );
service.addEvent(GAME_INPUT_MOVE_BACKWARDS,     io::KEY_S           );
service.addEvent(GAME_INPUT_MOVE_LEFT,          io::KEY_A           );
service.addEvent(GAME_INPUT_MOVE_RIGHT,         io::KEY_D           );
service.addEvent(GAME_INPUT_JUMP,               io::KEY_SPACE       );

// Primary input for an event
service.addEvent(GAME_INPUT_SHOOT,              io::MOUSE_LEFT      );
// Alternative input for the same event
service.addEvent(GAME_INPUT_SHOOT,              io::KEY_RETURN      );

service.addEvent(GAME_INPUT_RELOAD,             io::KEY_R           );
service.addEvent(GAME_INPUT_WEAPON_NEXT,        io::MOUSEWHEEL_UP   );
service.addEvent(GAME_INPUT_WEAPON_PREVIOUS,    io::MOUSEWHEEL_DOWN );

// ...

if (IsChangeGameControlSettings)
{
    // User can press any key or mouse button to change the key binding for the jump action
    while (!service.addEventKeyBinding(GAME_INPUT_JUMP))
        DrawPressAnyKeyInfo();
}

// ...

if (service.down(GAME_INPUT_MOVE_FORWARDS))
    MovePlayerForwards();

if (service.hit(GAME_INPUT_SHOOT))
    Shoot();

// ...
\endcode
\since Version 3.2 
*/
class SP_EXPORT InputService
{
    
    public:
        
        /* === Structures === */
        
        //! Event entry structure. Each event can have several entries (to have an alternative input).
        struct SEvent
        {
            SEvent() :
                Type    (io::INPUTTYPE_KEYBOARD ),
                KeyCode (0                      )
            {
            }
            ~SEvent()
            {
            }
            
            /* Operators */
            bool operator == (const SEvent &Other) const
            {
                return Type == Other.Type && KeyCode == Other.KeyCode;
            }
            
            /* Members */
            io::EInputTypes Type;   //!< Input type (keyboard, mouse, mouse-wheel, joystick).
            s32 KeyCode;            //!< Keycode number. Cast to io::EKeyCodes, io::EMouseKeyCodes, io::EMouseWheelMotions or io::EJoystickKeyCodes.
        };
        
        typedef std::vector<SEvent> TEventEntry;
        
        /* === Constructor & destructor === */
        
        InputService();
        virtual ~InputService();
        
        /* === Functions === */
        
        //! Sets the input count limitation for each entry. By default 2.
        void setMaxInputCount(u32 Count);
        
        //! Adds a new event entry. \see addEventKeyBinding
        void addEvent(s32 EventID);
        //! Adds a new event entry with the specified key. \see addEventKeyBinding
        void addEvent(s32 EventID, const io::EKeyCodes KeyCode);
        //! Adds a new event entry with the specified mouse button. \see addEventKeyBinding
        void addEvent(s32 EventID, const io::EMouseKeyCodes MouseKeyCode);
        //! Adds a new event entry with the specified mouse wheel motion. \see addEventKeyBinding
        void addEvent(s32 EventID, const io::EMouseWheelMotions MouseWheelMotion);
        //! Adds a new event entry with the specified joystick button. \see addEventKeyBinding
        void addEvent(s32 EventID, const io::EJoystickKeyCodes JoystickKeyCode);
        
        /**
        Adds a new event entry by key binding.
        \param[in] EventID Specifies the event ID number. This can be any integer number.
        It's recommended that you use your own enumerations for this.
        \param[in] Flags Specifies which input types are to be used. By default all (keyboard, mouse, mouse-wheel and joystick).
        \return False as long as no input has been detected. Once any key, mouse button,
        jostick button or mouse wheel motion has been detected the function returns true.
        */
        bool addEventKeyBinding(s32 EventID, s32 Flags = io::INPUTTYPE_ANY);
        
        //! Resets the specified event entry. After that it has no input information.
        void resetEvent(s32 EventID);
        //! Removes the specified event entry.
        void removeEvent(s32 EventID);
        
        //! Returns true if the specified event entry is active. Use this for a key-down, mouse-down, joystick-down and mouse-wheel checks.
        bool down(s32 EventID);
        //! Returns true if the specified event entry is active. Use this for a key-hit and mouse-hit checks.
        bool hit(s32 EventID);
        //! Returns true if the specified event entry is active. Use this for a key-released and mouse-released checks.
        bool released(s32 EventID);
        
        //! Returns the entry list for the specified event.
        TEventEntry getEventEntryList(s32 EventID) const;
        
    protected:
        
        /* === Functions === */
        
        void addEventEntry(s32 EventID, const SEvent &EventEntry);
        
        bool checkForKeyBinding(s32 EventID, s32 FirstIndex, s32 LastIndex);
        
        /* === Inline functions === */
        
        inline TEventEntry* getEvent(s32 EventID)
        {
            return &(EventEntryMap_[EventID]);
        }
        
        /* === Members === */
        
        u32 MaxInputCount_;
        
        std::map<s32, TEventEntry> EventEntryMap_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
