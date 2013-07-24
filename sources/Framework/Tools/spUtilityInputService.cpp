/*
 * Input service file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityInputService.hpp"

#ifdef SP_COMPILE_WITH_INPUTSERVICE


#include <boost/foreach.hpp>


namespace sp
{

extern io::InputControl* __spInputControl;

namespace tool
{


InputService::InputService() :
    MaxInputCount_(2)
{
}
InputService::~InputService()
{
}

void InputService::setMaxInputCount(u32 Count)
{
    if (Count < MaxInputCount_)
    {
        MaxInputCount_ = math::Max(1u, Count);
        
        for (std::map<s32, TEventEntry>::iterator it = EventEntryMap_.begin(); it != EventEntryMap_.end(); ++it)
        {
            TEventEntry* Entry = &(it->second);
            if (Entry->size() > Count)
                Entry->resize(Count);
        }
    }
    else
        MaxInputCount_ = Count;
}

void InputService::addEvent(s32 EventID)
{
    getEvent(EventID);
}

void InputService::addEvent(s32 EventID, const io::EKeyCodes KeyCode)
{
    SEvent EventEntry;
    {
        EventEntry.Type     = io::INPUTTYPE_KEYBOARD;
        EventEntry.KeyCode  = static_cast<s32>(KeyCode);
    }
    addEventEntry(EventID, EventEntry);
}

void InputService::addEvent(s32 EventID, const io::EMouseKeyCodes MouseKeyCode)
{
    SEvent EventEntry;
    {
        EventEntry.Type     = io::INPUTTYPE_MOUSE;
        EventEntry.KeyCode  = static_cast<s32>(MouseKeyCode);
    }
    addEventEntry(EventID, EventEntry);
}

void InputService::addEvent(s32 EventID, const io::EMouseWheelMotions MouseWheelMotion)
{
    SEvent EventEntry;
    {
        EventEntry.Type     = io::INPUTTYPE_MOUSEWHEEL;
        EventEntry.KeyCode  = static_cast<s32>(MouseWheelMotion);
    }
    addEventEntry(EventID, EventEntry);
}

void InputService::addEvent(s32 EventID, const io::EJoystickKeyCodes JoystickKeyCode)
{
    SEvent EventEntry;
    {
        EventEntry.Type     = io::INPUTTYPE_JOYSTICK;
        EventEntry.KeyCode  = static_cast<s32>(JoystickKeyCode);
    }
    addEventEntry(EventID, EventEntry);
}

#ifdef SP_COMPILE_WITH_XBOX360GAMEPAD

void InputService::addEvent(s32 EventID, const io::EGamePadButtons GamePadKeyCode, s32 Number)
{
    SEvent EventEntry;
    {
        EventEntry.Type     = io::INPUTTYPE_GAMEPAD;
        EventEntry.KeyCode  = static_cast<s32>(GamePadKeyCode);
        EventEntry.Number   = Number;
    }
    addEventEntry(EventID, EventEntry);
}

#endif

bool InputService::addEventKeyBinding(s32 EventID, s32 Flags)
{
    if (Flags & io::INPUTTYPE_KEYBOARD)
    {
        /* Check for keyboard */
        if ( checkForKeyBinding(EventID, io::KEY_BACK,      io::KEY_CAPITAL ) ||
             checkForKeyBinding(EventID, io::KEY_ESCAPE,    io::KEY_F24     ) ||
             checkForKeyBinding(EventID, io::KEY_NUMLOCK,   io::KEY_SCROLL  ) ||
             checkForKeyBinding(EventID, io::KEY_LSHIFT,    io::KEY_RMENU   ) ||
             checkForKeyBinding(EventID, io::KEY_PLUS,      io::KEY_PERIOD  ) ||
             checkForKeyBinding(EventID, io::KEY_EXPONENT,  io::KEY_EXPONENT) )
        {
            return true;
        }
    }
    
    if (Flags & io::INPUTTYPE_MOUSE)
    {
        /* Check for mouse buttons */
        for (s32 i = io::MOUSE_LEFT; i <= io::MOUSE_RIGHT; ++i)
        {
            if (__spInputControl->mouseDown(static_cast<io::EMouseKeyCodes>(i)))
            {
                addEvent(EventID, static_cast<io::EMouseKeyCodes>(i));
                return true;
            }
        }
    }
    
    if (Flags & io::INPUTTYPE_MOUSEWHEEL)
    {
        /* Check for mouse wheel motion */
        if (__spInputControl->getMouseWheel())
        {
            addEvent(EventID, static_cast<io::EMouseWheelMotions>(math::sgn(__spInputControl->getMouseWheel())));
            return true;
        }
    }
    
    if (Flags & io::INPUTTYPE_JOYSTICK)
    {
        /* Check for joystick buttons */
        for (s32 i = io::JOYSTICK_KEY1; i <= io::JOYSTICK_KEY16; ++i)
        {
            if (__spInputControl->joystickDown(static_cast<io::EJoystickKeyCodes>(i)))
            {
                addEvent(EventID, static_cast<io::EJoystickKeyCodes>(i));
                return true;
            }
        }
    }
    
    #ifdef SP_COMPILE_WITH_XBOX360GAMEPAD
    
    if (Flags & io::INPUTTYPE_GAMEPAD)
    {
        for (s32 i = 0; i < io::MAX_XBOX_CONTROLLERS; ++i)
        {
            io::XBox360GamePadPtr GamePad = __spInputControl->getXBox360GamePad(i);
            
            if (!GamePad->connected())
                continue;
            
            for (s32 j = 0; j < io::GAMEPAD_BUTTON_COUNT; ++j)
            {
                if (GamePad->buttonDown(static_cast<io::EGamePadButtons>(j)))
                {
                    addEvent(EventID, static_cast<io::EGamePadButtons>(j), i);
                    return true;
                }
            }
        }
    }
    
    #endif
    
    return false;
}

void InputService::resetEvent(s32 EventID)
{
    std::map<s32, TEventEntry>::iterator it = EventEntryMap_.find(EventID);
    
    if (it != EventEntryMap_.end())
        it->second.clear();
}
void InputService::removeEvent(s32 EventID)
{
    std::map<s32, TEventEntry>::iterator it = EventEntryMap_.find(EventID);
    
    if (it != EventEntryMap_.end())
        EventEntryMap_.erase(it);
}

bool InputService::down(s32 EventID)
{
    std::map<s32, TEventEntry>::iterator it = EventEntryMap_.find(EventID);
    
    if (it == EventEntryMap_.end())
        return false;
    
    foreach (const SEvent &Evt, it->second)
    {
        switch (Evt.Type)
        {
            case io::INPUTTYPE_KEYBOARD:
                if (__spInputControl->keyDown(static_cast<io::EKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSE:
                if (__spInputControl->mouseDown(static_cast<io::EMouseKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSEWHEEL:
                if (math::sgn(__spInputControl->getMouseWheel()) == Evt.KeyCode)
                    return true;
                break;
            case io::INPUTTYPE_JOYSTICK:
                if (__spInputControl->joystickDown(static_cast<io::EJoystickKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            #ifdef SP_COMPILE_WITH_XBOX360GAMEPAD
            case io::INPUTTYPE_GAMEPAD:
                if (__spInputControl->getXBox360GamePad(Evt.Number)->buttonDown(static_cast<io::EGamePadButtons>(Evt.KeyCode)))
                    return true;
                break;
            #endif
        }
    }
    
    return false;
}

bool InputService::hit(s32 EventID)
{
    std::map<s32, TEventEntry>::iterator it = EventEntryMap_.find(EventID);
    
    if (it == EventEntryMap_.end())
        return false;
    
    foreach (const SEvent &Evt, it->second)
    {
        switch (Evt.Type)
        {
            case io::INPUTTYPE_KEYBOARD:
                if (__spInputControl->keyHit(static_cast<io::EKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSE:
                if (__spInputControl->mouseHit(static_cast<io::EMouseKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSEWHEEL:
                if (math::sgn(__spInputControl->getMouseWheel()) == Evt.KeyCode)
                    return true;
                break;
            #ifdef SP_COMPILE_WITH_XBOX360GAMEPAD
            case io::INPUTTYPE_GAMEPAD:
                if (__spInputControl->getXBox360GamePad(Evt.Number)->buttonHit(static_cast<io::EGamePadButtons>(Evt.KeyCode)))
                    return true;
                break;
            #endif
        }
    }
    
    return false;
}

bool InputService::released(s32 EventID)
{
    std::map<s32, TEventEntry>::iterator it = EventEntryMap_.find(EventID);
    
    if (it == EventEntryMap_.end())
        return false;
    
    foreach (const SEvent &Evt, it->second)
    {
        switch (Evt.Type)
        {
            case io::INPUTTYPE_KEYBOARD:
                if (__spInputControl->keyReleased(static_cast<io::EKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSE:
                if (__spInputControl->mouseReleased(static_cast<io::EMouseKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            #ifdef SP_COMPILE_WITH_XBOX360GAMEPAD
            case io::INPUTTYPE_GAMEPAD:
                if (__spInputControl->getXBox360GamePad(Evt.Number)->buttonReleased(static_cast<io::EGamePadButtons>(Evt.KeyCode)))
                    return true;
                break;
            #endif
        }
    }
    
    return false;
}

InputService::TEventEntry InputService::getEventEntryList(s32 EventID) const
{
    std::map<s32, TEventEntry>::const_iterator it = EventEntryMap_.find(EventID);
    return it != EventEntryMap_.end() ? it->second : TEventEntry();
}


/*
 * ======= Protected: =======
 */

void InputService::addEventEntry(s32 EventID, const SEvent &EventEntry)
{
    TEventEntry* Event = getEvent(EventID);
    
    /* Check if this entry already exists in this event */
    for (u32 i = 0; i < Event->size(); ++i)
    {
        if ((*Event)[i] == EventEntry)
        {
            if (i > 0)
            {
                /* Move existing entry at the top of the list */
                for (; i > 0; --i)
                    (*Event)[i] = (*Event)[i - 1];
                (*Event)[0] = EventEntry;
            }
            return;
        }
    }
    
    /* Resize entry list and move entries forwards */
    if (Event->size() < MaxInputCount_)
        Event->resize(Event->size() + 1);
    else
        io::Log::warning("Overwriting input events (limit is " + io::stringc(MaxInputCount_) + ")");
    
    for (u32 i = Event->size() - 1; i > 0; --i)
        (*Event)[i] = (*Event)[i - 1];
    (*Event)[0] = EventEntry;
}

bool InputService::checkForKeyBinding(s32 EventID, s32 FirstIndex, s32 LastIndex)
{
    for (s32 i = FirstIndex; i <= LastIndex; ++i)
    {
        if (__spInputControl->keyDown(static_cast<io::EKeyCodes>(i)))
        {
            addEvent(EventID, static_cast<io::EKeyCodes>(i));
            return true;
        }
    }
    return false;
}


/*
 * SEvent structure
 */

InputService::SEvent::SEvent() :
    Type    (io::INPUTTYPE_KEYBOARD ),
    KeyCode (0                      )
    #ifdef SP_COMPILE_WITH_XBOX360GAMEPAD
    ,Number (0                      )
    #endif
{
}
InputService::SEvent::~SEvent()
{
}

bool InputService::SEvent::operator == (const SEvent &Other) const
{
    return Type == Other.Type && KeyCode == Other.KeyCode;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
