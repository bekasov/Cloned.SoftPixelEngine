/*
 * Storyboard active consequence header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_ACTIVE_CONSEQUENCE_H__
#define __SP_STORYBOARD_ACTIVE_CONSEQUENCE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include "Base/spBaseObject.hpp"
#include "Framework/Tools/spStoryboardConsequence.hpp"


namespace sp
{
namespace tool
{


class SP_EXPORT StoryboardActiveConsequence : public BaseObject, public StoryboardConsequence
{
    
    public:
        
        virtual ~StoryboardActiveConsequence()
        {
        }
        
    protected:
        
        StoryboardActiveConsequence() :
            BaseObject              (),
            StoryboardConsequence   ()
        {
        }
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
