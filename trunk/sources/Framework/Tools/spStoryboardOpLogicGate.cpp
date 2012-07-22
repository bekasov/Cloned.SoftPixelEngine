/*
 * Storyboard operator logic gate file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spStoryboardOpLogicGate.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


StoryboardOpLogicGate::StoryboardOpLogicGate(const EStoryboardLogicGates Type) :
    StoryboardOperator  (       ),
    Type_               (Type   )
{
}
StoryboardOpLogicGate::~StoryboardOpLogicGate()
{
}

bool StoryboardOpLogicGate::isActive() const
{
    s32 Count = 0;
    
    switch (Type_)
    {
        case LOGICGATE_AND:
            foreach (StoryboardTrigger* Trigger, Triggers_)
            {
                if (!Trigger->isActive())
                    return false;
            }
            return true;
            
        case LOGICGATE_NAND:
            foreach (StoryboardTrigger* Trigger, Triggers_)
            {
                if (!Trigger->isActive())
                    return true;
            }
            return false;
            
        case LOGICGATE_OR:
            foreach (StoryboardTrigger* Trigger, Triggers_)
            {
                if (Trigger->isActive())
                    return true;
            }
            return false;
            
        case LOGICGATE_NOR:
            foreach (StoryboardTrigger* Trigger, Triggers_)
            {
                if (Trigger->isActive())
                    return false;
            }
            return true;
            
        case LOGICGATE_XOR:
            foreach (StoryboardTrigger* Trigger, Triggers_)
            {
                if (Trigger->isActive())
                    ++Count;
            }
            return Count % 2 == 1;
            
        case LOGICGATE_XNOR:
            foreach (StoryboardTrigger* Trigger, Triggers_)
            {
                if (Trigger->isActive())
                    ++Count;
            }
            return Count % 2 == 0;
    }
    
    return false;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
