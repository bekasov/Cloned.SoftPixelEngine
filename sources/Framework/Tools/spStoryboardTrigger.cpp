/*
 * Storyboard trigger file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spStoryboardTrigger.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Framework/Tools/spStoryboard.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


Trigger::Trigger() :
    IsTriggered_(false)
{
}
Trigger::~Trigger()
{
    /* Clear all child triggers */
    clearTriggers();
    
    /* Untrigger */
    if (IsTriggered_)
    {
        IsTriggered_ = false;
        onUntriggered();
    }
}

bool Trigger::triggeredParents() const
{
    if (Parents_.empty())
        return true;
    foreach (Trigger* Parent, Parents_)
    {
        if (Parent->triggered())
            return true;
    }
    return false;
}

bool Trigger::needLoopUpdate() const
{
    return false;
}

bool Trigger::canTrigger() const
{
    return true;
}

void Trigger::trigger()
{
    if (!IsTriggered_ && canTrigger() && triggeredParents())
    {
        IsTriggered_ = true;
        onTriggered();
    }
}

void Trigger::untrigger()
{
    if ( IsTriggered_ && ( Parents_.empty() || !triggeredParents() ) )
    {
        IsTriggered_ = false;
        onUntriggered();
    }
}

void Trigger::connect(Trigger* ChildTrigger)
{
    if (ChildTrigger && !MemoryManager::hasElement(Children_, ChildTrigger))
    {
        Children_.push_back(ChildTrigger);
        ChildTrigger->Parents_.push_back(this);
    }
}

void Trigger::disconnect(Trigger* ChildTrigger)
{
    if (ChildTrigger)
    {
        Trigger* ThisTrigger = this;
        MemoryManager::removeElement(Children_, ChildTrigger);
        MemoryManager::removeElement(ChildTrigger->Parents_, ThisTrigger);
    }
}

void Trigger::clearTriggers()
{
    Trigger* ThisTrigger = this;
    foreach (Trigger* Child, Children_)
        MemoryManager::removeElement(Child->Parents_, ThisTrigger);
    Children_.clear();
}

void Trigger::onTriggered()
{
    addToLoopUpdate();
    triggerChildren();
}

void Trigger::onUntriggered()
{
    removeFromLoopUpdate();
    untriggerChildren();
}

void Trigger::onRunning()
{
    // do nothing
}


/*
 * ======= Protected: =======
 */

void Trigger::triggerChildren()
{
    foreach (Trigger* Child, Children_)
        Child->trigger();
}
void Trigger::untriggerChildren()
{
    foreach (Trigger* Child, Children_)
        Child->untrigger();
}

void Trigger::addToLoopUpdate()
{
    if (needLoopUpdate() && Storyboard::getActive())
        Storyboard::getActive()->addLoopUpdate(this);
}
void Trigger::removeFromLoopUpdate()
{
    if (needLoopUpdate() && Storyboard::getActive())
        Storyboard::getActive()->removeLoopUpdate(this);
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
