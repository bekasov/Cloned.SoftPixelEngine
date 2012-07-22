/*
 * Storyboard consequence header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_STORYBOARD_CONSEQUENCE_H__
#define __SP_STORYBOARD_CONSEQUENCE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


namespace sp
{
namespace tool
{


class SP_EXPORT StoryboardConsequence
{
    
    public:
        
        virtual ~StoryboardConsequence()
        {
        }
        
        /* Functions */
        
        //! Runs the consequence effect.
        virtual void run() = 0;
        
    protected:
        
        StoryboardConsequence()
        {
        }
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
