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
    IsTriggered_(false),
    WasActive_  (false)
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

#if 0

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

#else

bool Trigger::active() const
{
    if (!triggered())
        return false;
    foreach (Trigger* Parent, Parents_)
    {
        if (!Parent->active())
            return false;
    }
    return true;
}

#endif

bool Trigger::needLoopUpdate() const
{
    return false;
}

bool Trigger::canTrigger() const
{
    return true;
}

#if 0

void Trigger::trigger()
{
    if (canTrigger())
    {
        /* Check if object can be triggered */
        if (!IsTriggered_ && triggeredParents())
        {
            IsTriggered_ = true;
            onTriggered();
        }
        /* Check if object is already triggered but parents have changed their states */
        else if (IsTriggered_ && !triggeredParents())
        {
            IsTriggered_ = false;
            onUntriggered();
        }
    }
}

void Trigger::untrigger()
{
    if (IsTriggered_ && ( Parents_.empty() || !triggeredParents() ) )
    {
        IsTriggered_ = false;
        onUntriggered();
    }
}

#else

void Trigger::trigger()
{
    if (!IsTriggered_)
    {
        IsTriggered_ = true;
        updateStates();
    }
}

void Trigger::untrigger()
{
    if (IsTriggered_)
    {
        IsTriggered_ = false;
        updateStates();
    }
}

#endif

void Trigger::connect(Trigger* ChildTrigger)
{
    if (ChildTrigger && !MemoryManager::hasElement(Children_, ChildTrigger))
    {
        Children_.push_back(ChildTrigger);
        ChildTrigger->Parents_.push_back(this);
        
        updateStates();
    }
}

void Trigger::disconnect(Trigger* ChildTrigger)
{
    if (ChildTrigger)
    {
        Trigger* ThisTrigger = this;
        MemoryManager::removeElement(Children_, ChildTrigger);
        MemoryManager::removeElement(ChildTrigger->Parents_, ThisTrigger);
        
        updateStates();
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
    //addToLoopUpdate();
    //triggerChildren();
}

void Trigger::onUntriggered()
{
    //removeFromLoopUpdate();
    //untriggerChildren();
}

void Trigger::onRunning()
{
    // do nothing
}


/*
 * ======= Protected: =======
 */

#if 0

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

#endif

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

void Trigger::updateStates()
{
    /* Check if this object is now active */
    bool NowActive = active();
    
    if (WasActive_ != NowActive)
    {
        if (NowActive)
        {
            addToLoopUpdate();
            onTriggered();
        }
        else
        {
            removeFromLoopUpdate();
            onUntriggered();
        }
        WasActive_ = NowActive;
    }
    
    /* Update children states */
    foreach (Trigger* Child, Children_)
        Child->updateStates();
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
