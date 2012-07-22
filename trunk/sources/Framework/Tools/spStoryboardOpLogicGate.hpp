/*
 * Storyboard operator logic gate header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_LOGICGATE_H__
#define __SP_STORYBOARD_LOGICGATE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Framework/Tools/spStoryboardOperator.hpp"


namespace sp
{
namespace tool
{


//! Logic gates are used for combinatory logic in the storyboard.
enum EStoryboardLogicGates
{
    LOGICGATE_AND,
    LOGICGATE_NAND,
    LOGICGATE_OR,
    LOGICGATE_NOR,
    LOGICGATE_XOR,
    LOGICGATE_XNOR,
};


class SP_EXPORT StoryboardOpLogicGate : public StoryboardOperator
{
    
    public:
        
        StoryboardOpLogicGate(const EStoryboardLogicGates Type);
        ~StoryboardOpLogicGate();
        
        /* Functions */
        
        bool isActive() const;
        
        /* Inline functions */
        
        inline void setType(const EStoryboardLogicGates Type)
        {
            Type_ = Type;
        }
        inline EStoryboardLogicGates getType() const
        {
            return Type_;
        }
        
    private:
        
        /* Members */
        
        EStoryboardLogicGates Type_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
