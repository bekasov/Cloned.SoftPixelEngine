/*
 * Animation interface file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spAnimation.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


Animation::Animation(const EAnimationTypes Type) :
    MinFrame_   (0      ),
    MaxFrame_   (0      ),
    Type_       (Type   )
{
}
Animation::~Animation()
{
    clearSceneNodes();
}

bool Animation::play(const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame)
{
    if (getKeyframeCount() >= 2)
        return Playback_.play(Mode, getValidFrame(FirstFrame), getValidFrame(LastFrame));
    return false;
}

void Animation::pause(bool isPaused)
{
    Playback_.pause(isPaused);
}

void Animation::stop()
{
    Playback_.stop();
}

void Animation::setFrame(u32 Index)
{
    if (!getKeyframeCount())
        return;
    
    /* Setup current frame */
    if (Index >= getKeyframeCount())
        Index = getKeyframeCount();
    
    Playback_.setFrame(Index);
}

bool Animation::interpolateRange(u32 FirstFrame, u32 LastFrame, f32 Interpolation)
{
    if (getKeyframeCount() < 2)
        return false;
    
    /* Make playback interpolation */
    FirstFrame  = getValidFrame(FirstFrame);
    LastFrame   = getValidFrame(LastFrame);
    
    if (!AnimationPlayback::interpolateRange(FirstFrame, LastFrame, Interpolation))
        return false;
    
    /* Interpolate between the determined two frames */
    interpolate(FirstFrame, LastFrame, Interpolation);
    
    return true;
}

void Animation::addSceneNode(SceneNode* Object)
{
    if (Object)
    {
        SceneNodes_.push_back(Object);
        Object->AnimationList_.push_back(this);
    }
}

void Animation::removeSceneNode(SceneNode* Object)
{
    if (Object)
    {
        MemoryManager::removeElement(SceneNodes_, Object);
        Animation* Anim = this;
        MemoryManager::removeElement(Object->AnimationList_, Anim);
    }
}

void Animation::clearSceneNodes()
{
    Animation* Anim = this;
    
    foreach (SceneNode* Object, SceneNodes_)
        MemoryManager::removeElement(Object->AnimationList_, Anim);
    
    SceneNodes_.clear();
}


/*
 * ======= Protected: =======
 */

void Animation::updatePlayback(f32 Speed)
{
    /* Interpolate between the current and the next frame */
    interpolate(Playback_.getFrame(), static_cast<u32>(Playback_.getNextFrame()), Playback_.getInterpolation());
    
    Playback_.update(Speed);
}

u32 Animation::getValidFrame(u32 Index) const
{
    if (!getKeyframeCount())
        return 0;
    if (Index >= getKeyframeCount())
        return getKeyframeCount() - 1;
    return Index;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
