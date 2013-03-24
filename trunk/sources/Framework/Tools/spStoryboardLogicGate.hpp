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


#include "Framework/Tools/spStoryboardTrigger.hpp"


namespace sp
{
namespace tool
{


//! Logic gates are used for combinatory logic in the storyboard.
enum ELogicGates
{
    LOGICGATE_AND,
    LOGICGATE_NAND,
    LOGICGATE_OR,
    LOGICGATE_NOR,
    LOGICGATE_XOR,
    LOGICGATE_XNOR,
};


class SP_EXPORT LogicGate : public Trigger
{
    
    public:
        
        LogicGate(const ELogicGates Type);
        ~LogicGate();
        
        /* === Functions === */
        
        //bool triggeredParents() const;
        bool active() const;
        
        bool needLoopUpdate() const;
        
        /* === Inline functions === */
        
        inline void setType(const ELogicGates Type)
        {
            Type_ = Type;
        }
        inline ELogicGates getType() const
        {
            return Type_;
        }
        
    private:
        
        /* === Members === */
        
        ELogicGates Type_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
