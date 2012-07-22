/*
 * Storyboard trigger header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_TRIGGER_H__
#define __SP_STORYBOARD_TRIGGER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include <vector>


namespace sp
{
namespace tool
{


class StoryboardOperator;
class StoryboardConsequence;

class SP_EXPORT StoryboardTrigger
{
    
    public:
        
        virtual ~StoryboardTrigger();
        
        /* Functions */
        
        //! Returns true if the trigger is active.
        virtual bool isActive() const = 0;
        
        //! Activation 'callback' function.
        virtual void activate();
        
        virtual void addConsequence(StoryboardOperator* Consequence);
        virtual void removeConsequence(StoryboardOperator* Consequence);
        
        virtual void addConsequence(StoryboardConsequence* Consequence);
        virtual void removeConsequence(StoryboardConsequence* Consequence);
        
        /* Inline functions */
        
        inline const std::vector<StoryboardConsequence*>& getConsequenceList() const
        {
            return Consequences_;
        }
        
    protected:
        
        StoryboardTrigger();
        
        /* Members */
        
        std::vector<StoryboardConsequence*> Consequences_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
