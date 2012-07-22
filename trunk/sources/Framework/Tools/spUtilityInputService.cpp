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
namespace tool
{


InputService::InputService(io::InputControl* Controller) :
    Controller_     (Controller ),
    MaxInputCount_  (2          )
{
    if (!Controller_)
        io::Log::error("Invalid input controller for input service");
}
InputService::~InputService()
{
}

void InputService::setMaxInputCount(s32 Count)
{
    if (Count < MaxInputCount_)
    {
        MaxInputCount_ = math::Max(1, Count);
        
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
            if (Controller_->mouseDown(static_cast<io::EMouseKeyCodes>(i)))
            {
                addEvent(EventID, static_cast<io::EMouseKeyCodes>(i));
                return true;
            }
        }
    }
    
    if (Flags & io::INPUTTYPE_MOUSEWHEEL)
    {
        /* Check for mouse wheel motion */
        if (Controller_->getMouseWheel())
        {
            addEvent(EventID, static_cast<io::EMouseWheelMotions>(math::Sgn(Controller_->getMouseWheel())));
            return true;
        }
    }
    
    if (Flags & io::INPUTTYPE_JOYSTICK)
    {
        /* Check for joystick buttons */
        for (s32 i = io::JOYSTICK_KEY1; i <= io::JOYSTICK_KEY16; ++i)
        {
            if (Controller_->joystickDown(static_cast<io::EJoystickKeyCodes>(i)))
            {
                addEvent(EventID, static_cast<io::EJoystickKeyCodes>(i));
                return true;
            }
        }
    }
    
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
                if (Controller_->keyDown(static_cast<io::EKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSE:
                if (Controller_->mouseDown(static_cast<io::EMouseKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSEWHEEL:
                if (math::Sgn(Controller_->getMouseWheel()) == Evt.KeyCode);
                    return true;
                break;
            case io::INPUTTYPE_JOYSTICK:
                if (Controller_->joystickDown(static_cast<io::EJoystickKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
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
                if (Controller_->keyHit(static_cast<io::EKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSE:
                if (Controller_->mouseHit(static_cast<io::EMouseKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
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
                if (Controller_->keyReleased(static_cast<io::EKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
            case io::INPUTTYPE_MOUSE:
                if (Controller_->mouseReleased(static_cast<io::EMouseKeyCodes>(Evt.KeyCode)))
                    return true;
                break;
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
    
    /* Check if this entry already exists in another event */
    for (std::map<s32, TEventEntry>::iterator it = EventEntryMap_.begin(); it != EventEntryMap_.end(); ++it)
    {
        /* Don't compare with the current event */
        if (it->first == EventID)
            continue;
        
        for (TEventEntry::iterator itEntry = it->second.begin(); itEntry != it->second.end(); ++itEntry)
        {
            /* Check if the new entry is equal to the current one */
            if (*itEntry == EventEntry)
            {
                /* Remove entry from old event */
                it->second.erase(itEntry);
                
                /* Break both for-loops */
                it = EventEntryMap_.end();
                break;
            }
        }
    }
    
    /* Resize entry list and move entries forwards */
    if (Event->size() < MaxInputCount_)
        Event->resize(Event->size() + 1);
    
    for (u32 i = Event->size() - 1; i > 0; --i)
        (*Event)[i] = (*Event)[i - 1];
    (*Event)[0] = EventEntry;
}

bool InputService::checkForKeyBinding(s32 EventID, s32 FirstIndex, s32 LastIndex)
{
    for (s32 i = FirstIndex; i <= LastIndex; ++i)
    {
        if (Controller_->keyDown(static_cast<io::EKeyCodes>(i)))
        {
            addEvent(EventID, static_cast<io::EKeyCodes>(i));
            return true;
        }
    }
    return false;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
