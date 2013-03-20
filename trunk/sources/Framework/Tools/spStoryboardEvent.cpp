/*
 * Storyboard operator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spStoryboardEvent.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


Event::Event() :
    Trigger()
{
}
Event::~Event()
{
}


EventTimer::EventTimer(u64 Duration) :
    Event   (           ),
    Timer_  (Duration   )
{
}
EventTimer::~EventTimer()
{
}

void EventTimer::update()
{
    if (Timer_.finish())
        trigger();
}

void EventTimer::onTriggered()
{
    Trigger::onTriggered();
    
    Timer_.reset();
    untrigger();
}


TriggerCounter::TriggerCounter(u32 Counter) :
    Trigger     (       ),
    OrigCounter_(Counter),
    Counter_    (Counter)
{
}
TriggerCounter::~TriggerCounter()
{
}

bool TriggerCounter::canTrigger() const
{
    return Counter_ > 0;
}

void TriggerCounter::onTriggered()
{
    Trigger::onTriggered();
    
    if (Counter_ > 0)
        --Counter_;
}

void TriggerCounter::reset()
{
    Counter_ = OrigCounter_;
}
void TriggerCounter::reset(u32 Counter)
{
    OrigCounter_ = Counter;
    reset();
}


TriggerSwitch::TriggerSwitch(u32 Selection) :
    Trigger     (           ),
    Selection_  (Selection  )
{
}
TriggerSwitch::~TriggerSwitch()
{
}

void TriggerSwitch::onTriggered()
{
    if (!getChildList().empty())
    {
        /* Untrigger previous selection */
        if (Selection_ < getChildList().size())
            getChildList()[Selection_]->untrigger();
        
        /* Increment and clamp selection index */
        ++Selection_;
        if (Selection_ >= getChildList().size())
            Selection_ = 0;
        
        /* Trigger selection only */
        getChildList()[Selection_]->trigger();
    }
}

void TriggerSwitch::onUntriggered()
{
    if (!getChildList().empty())
    {
        /* Clamp selection index */
        if (Selection_ >= getChildList().size())
            Selection_ = 0;
        
        /* Untrigger selection only */
        getChildList()[Selection_]->untrigger();
    }
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
