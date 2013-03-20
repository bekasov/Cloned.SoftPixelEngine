/*
 * Storyboard operator logic gate file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spStoryboardLogicGate.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


LogicGate::LogicGate(const ELogicGates Type) :
    Trigger (       ),
    Type_   (Type   )
{
}
LogicGate::~LogicGate()
{
}

bool LogicGate::triggeredParents() const
{
    s32 Count = 0;
    
    switch (Type_)
    {
        case LOGICGATE_AND:
            foreach (Trigger* Parent, getParentList())
            {
                if (!Parent->triggered())
                    return false;
            }
            return true;
            
        case LOGICGATE_NAND:
            foreach (Trigger* Parent, getParentList())
            {
                if (!Parent->triggered())
                    return true;
            }
            return false;
            
        case LOGICGATE_OR:
            foreach (Trigger* Parent, getParentList())
            {
                if (Parent->triggered())
                    return true;
            }
            return false;
            
        case LOGICGATE_NOR:
            foreach (Trigger* Parent, getParentList())
            {
                if (Parent->triggered())
                    return false;
            }
            return true;
            
        case LOGICGATE_XOR:
            foreach (Trigger* Parent, getParentList())
            {
                if (Parent->triggered())
                    ++Count;
            }
            return Count % 2 == 1;
            
        case LOGICGATE_XNOR:
            foreach (Trigger* Parent, getParentList())
            {
                if (Parent->triggered())
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
