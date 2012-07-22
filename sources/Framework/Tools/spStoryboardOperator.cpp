/*
 * Storyboard operator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spStoryboardOperator.hpp"

#ifdef SP_COMPILE_WITH_STORYBOARD


#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


StoryboardOperator::StoryboardOperator() :
    StoryboardTrigger       (),
    StoryboardConsequence   ()
{
}
StoryboardOperator::~StoryboardOperator()
{
}

void StoryboardOperator::run()
{
    if (isActive())
    {
        foreach (StoryboardConsequence* Consequence, Consequences_)
            Consequence->run();
    }
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
