/*
 * Storyboard file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spStoryboard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


Storyboard* Storyboard::Active_ = 0;

Storyboard::Storyboard()
{
    if (!Storyboard::getActive())
        Storyboard::setActive(this);
}
Storyboard::~Storyboard()
{
    clearTriggers();
    clearEvents();
}

void Storyboard::update()
{
    /* Update all active triggers */
    foreach (Trigger* Obj, ActiveTriggers_)
        Obj->onRunning();
    
    /* Update all events */
    foreach (Event* Obj, Events_)
        Obj->update();
}

void Storyboard::deleteTrigger(Trigger* Obj)
{
    MemoryManager::removeElement(Triggers_, Obj, true);
}
void Storyboard::clearTriggers()
{
    MemoryManager::deleteList(Triggers_);
}

void Storyboard::deleteEvent(Event* Obj)
{
    MemoryManager::removeElement(Events_, Obj, true);
}
void Storyboard::clearEvents()
{
    MemoryManager::deleteList(Events_);
}

Storyboard* Storyboard::getActive()
{
    return Active_;
}
void Storyboard::setActive(Storyboard* ActiveStoryboard)
{
    Active_ = ActiveStoryboard;
}


/*
 * ======= Private: =======
 */

void Storyboard::addLoopUpdate(Trigger* Obj)
{
    if (Obj && !MemoryManager::hasElement(ActiveTriggers_, Obj))
        ActiveTriggers_.push_back(Obj);
}
void Storyboard::removeLoopUpdate(Trigger* Obj)
{
    MemoryManager::removeElement(ActiveTriggers_, Obj);
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
