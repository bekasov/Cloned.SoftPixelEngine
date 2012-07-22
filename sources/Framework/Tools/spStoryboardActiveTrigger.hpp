/*
 * Storyboard active trigger header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_ACTIVE_TRIGGER_H__
#define __SP_STORYBOARD_ACTIVE_TRIGGER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Base/spBaseObject.hpp"
#include "Framework/Tools/spStoryboardTrigger.hpp"


namespace sp
{
namespace tool
{


class SP_EXPORT StoryboardActiveTrigger : public BaseObject, public StoryboardTrigger
{
    
    public:
        
        virtual ~StoryboardActiveTrigger()
        {
        }
        
    protected:
        
        StoryboardActiveTrigger() :
            BaseObject          (),
            StoryboardTrigger   ()
        {
        }
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
