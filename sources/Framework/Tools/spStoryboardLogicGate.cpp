/*
 * Storyboard operator logic gate file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spStoryboardLogicGate.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include <boost/foreach.hpp>

#if 1//!!!
#   include "Base/spInputOutputLog.hpp"
#endif


namespace sp
{
namespace tool
{


LogicGate::LogicGate(const ELogicGates Type) :
    Trigger (       ),
    Type_   (Type   )
{
    trigger();
}
LogicGate::~LogicGate()
{
}

//bool LogicGate::triggeredParents() const
bool LogicGate::active() const
{
    s32 Count = 0;
    
    switch (Type_)
    {
        case LOGICGATE_AND:
            foreach (Trigger* Parent, getParentList())
            {
                if (!Parent->active())
                    return false;
            }
            return true;
            
        case LOGICGATE_NAND:
            foreach (Trigger* Parent, getParentList())
            {
                if (!Parent->active())
                    return true;
            }
            return false;
            
        case LOGICGATE_OR:
            foreach (Trigger* Parent, getParentList())
            {
                if (Parent->active())
                    return true;
            }
            return false;
            
        case LOGICGATE_NOR:
            foreach (Trigger* Parent, getParentList())
            {
                if (Parent->active())
                    return false;
            }
            return true;
            
        case LOGICGATE_XOR:
            foreach (Trigger* Parent, getParentList())
            {
                if (Parent->active())
                    ++Count;
            }
            return Count % 2 == 1;
            
        case LOGICGATE_XNOR:
            foreach (Trigger* Parent, getParentList())
            {
                if (Parent->active())
                    ++Count;
            }
            return Count % 2 == 0;
    }
    
    return false;
}

bool LogicGate::needLoopUpdate() const
{
    return false;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
