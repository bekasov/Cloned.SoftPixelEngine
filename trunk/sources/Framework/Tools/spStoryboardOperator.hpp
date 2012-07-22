/*
 * Storyboard trigger header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_OPERATOR_H__
#define __SP_STORYBOARD_OPERATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Framework/Tools/spStoryboardTrigger.hpp"
#include "Framework/Tools/spStoryboardConsequence.hpp"

#include <vector>


namespace sp
{
namespace tool
{


/**
A storyboard operator connects events and facts with a consequence.
\see StoryboardEvent, StoryboardFact, StoryboardConsequence
\since Version 3.2
*/
class SP_EXPORT StoryboardOperator : public StoryboardTrigger, public StoryboardConsequence
{
    
    public:
        
        virtual ~StoryboardOperator();
        
        /* Functions */
        
        void run();
        
        /* Inline functions */
        
        inline const std::vector<StoryboardTrigger*>& getTriggerList() const
        {
            return Triggers_;
        }
        
    protected:
        
        friend class StoryboardTrigger;
        
        StoryboardOperator();
        
        /* Members */
        
        std::vector<StoryboardTrigger*> Triggers_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
