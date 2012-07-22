/*
 * Storyboard trigger file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spStoryboardTrigger.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Framework/Tools/spStoryboardConsequence.hpp"
#include "Framework/Tools/spStoryboardOperator.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


StoryboardTrigger::StoryboardTrigger()
{
}
StoryboardTrigger::~StoryboardTrigger()
{
}

void StoryboardTrigger::activate()
{
    foreach (StoryboardConsequence* Consequence, Consequences_)
        Consequence->run();
}

void StoryboardTrigger::addConsequence(StoryboardOperator* Consequence)
{
    if (Consequence)
    {
        Consequence->Triggers_.push_back(this);
        Consequences_.push_back(Consequence);
    }
}
void StoryboardTrigger::removeConsequence(StoryboardOperator* Consequence)
{
    if (Consequence)
    {
        StoryboardTrigger* ThisTrigger = this;
        MemoryManager::removeElement(Consequence->Triggers_, ThisTrigger);
        
        StoryboardConsequence* OpConseq = Consequence;
        MemoryManager::removeElement(Consequences_, OpConseq);
    }
}

void StoryboardTrigger::addConsequence(StoryboardConsequence* Consequence)
{
    if (Consequence)
        Consequences_.push_back(Consequence);
}
void StoryboardTrigger::removeConsequence(StoryboardConsequence* Consequence)
{
    MemoryManager::removeElement(Consequences_, Consequence);
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
