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
    Type_           (Type               ),
    Mode_           (PLAYBACK_ONESHOT   ),
    hasStarted_     (false              ),
    isPlaying_      (false              ),
    Frame_          (0                  ),
    NextFrame_      (0                  ),
    Interpolation_  (0.0f               ),
    FirstFrame_     (0                  ),
    LastFrame_      (0                  ),
    MinFrame_       (0                  ),
    MaxFrame_       (0                  ),
    Speed_          (1.0f               ),
    RepeatCount_    (0                  )
{
}
Animation::~Animation()
{
    clearSceneNodes();
}

bool Animation::play(const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame)
{
    /* Don't play animation if first- and last frame are equal or there are no keyframes */
    if (FirstFrame == LastFrame || getKeyframeCount() < 2)
        return false;
    
    /* Setup animation playback */
    Mode_           = Mode;
    
    hasStarted_     = true;
    isPlaying_      = true;
    
    FirstFrame_     = getValidFrame(FirstFrame);
    LastFrame_      = getValidFrame(LastFrame);
    
    Frame_          = FirstFrame_;
    RepeatCount_    = 0;
    
    /* Setup initial next frame */
    if (LastFrame_ >= FirstFrame_)
        NextFrame_ = FirstFrame_ + 1;
    else
        NextFrame_ = FirstFrame_ - 1;
    
    return true;
}

bool Animation::play(u32 SeqId)
{
    std::map<u32, SAnimSequence>::iterator it = Sequences_.find(SeqId);
    
    if (it != Sequences_.end())
    {
        setSpeed(it->second.Speed);
        return play(it->second.Mode, it->second.FirstFrame, it->second.LastFrame);
    }
    
    return false;
}

void Animation::pause(bool isPaused)
{
    if (hasStarted_)
        isPlaying_ = !isPaused;
}

void Animation::stop()
{
    if (hasStarted_)
    {
        /* Reset animation state */
        isPlaying_      = false;
        hasStarted_     = false;
        Frame_          = 0;
        NextFrame_      = 0;
        RepeatCount_    = 0;
    }
}

void Animation::setFrame(u32 Index)
{
    if (!getKeyframeCount() || Frame_ == Index)
        return;
    
    /* Setup current frame */
    if (Index >= getKeyframeCount())
        Frame_ = getKeyframeCount();
    else
        Frame_ = Index;
    
    /* Setup next frame */
    if (playing())
    {
        if (LastFrame_ > FirstFrame_)
        {
            if (Frame_ >= LastFrame_)
                stop();
            else
                NextFrame_ = Frame_ + 1;
        }
        else
        {
            if (Frame_ <= LastFrame_)
                stop();
            else
                NextFrame_ = Frame_ - 1;
        }
    }
}

void Animation::setupManualAnimation(SceneNode* Node)
{
    // do nothing
}

void Animation::interpolateSequence(u32 FirstFrame, u32 LastFrame, f32 Interpolation)
{
    if (getKeyframeCount() < 2 || FirstFrame >= LastFrame)
        return;
    
    /* Get frame range */
    math::Clamp(Interpolation, 0.0f, 1.0f);
    
    FirstFrame  = getValidFrame(FirstFrame);
    LastFrame   = getValidFrame(LastFrame);
    
    const u32 From  = FirstFrame + static_cast<u32>(Interpolation * (LastFrame - FirstFrame));
    const u32 To    = From + 1;
    
    /* Get final interpolation factor */
    Interpolation *= (LastFrame - FirstFrame);
    Interpolation -= (From - FirstFrame);
    
    /* Interpolate between the determined two frames */
    interpolate(From, To, Interpolation);
}

bool Animation::addSequence(
    u32 SeqId, const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame, f32 Speed)
{
    std::map<u32, SAnimSequence>::iterator it = Sequences_.find(SeqId);
    
    if (it == Sequences_.end())
    {
        SAnimSequence Sequence;
        {
            Sequence.Mode       = Mode;
            Sequence.FirstFrame = FirstFrame;
            Sequence.LastFrame  = LastFrame;
            Sequence.Speed      = Speed;
        }
        Sequences_[SeqId] = Sequence;
        
        return true;
    }
    
    return false;
}

bool Animation::removeSequence(u32 SeqId)
{
    std::map<u32, SAnimSequence>::iterator it = Sequences_.find(SeqId);
    
    if (it != Sequences_.end())
    {
        Sequences_.erase(it);
        return true;
    }
    
    return false;
}

void Animation::clearSequences()
{
    Sequences_.clear();
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
    interpolate(Frame_, static_cast<u32>(NextFrame_), Interpolation_);
    
    /* Increment the interpolation */
    Interpolation_ += Speed;
    
    while (Interpolation_ > 1.0f)
    {
        Interpolation_ -= 1.0f;
        
        /* Adopt the previous frame to the current one */
        Frame_ = NextFrame_;
        
        /* Increment or decrement the next frame */
        if (LastFrame_ >= FirstFrame_)
            ++NextFrame_;
        else
            --NextFrame_;
        
        /* Check if one-loop animation has already done */
        if (Mode_ == PLAYBACK_ONELOOP && Frame_ == FirstFrame_)
        {
            stop();
            return;
        }
        
        /* Check if current frame arrived the last frame */
        if (Frame_ == LastFrame_)
            checkAnimationEnding();
    }
}

u32 Animation::getValidFrame(u32 Index) const
{
    if (!getKeyframeCount())
        return 0;
    if (Index >= getKeyframeCount())
        return getKeyframeCount() - 1;
    return Index;
}


/*
 * ======= Private: =======
 */

void Animation::checkAnimationEnding()
{
    ++RepeatCount_;
    
    switch (Mode_)
    {
        case PLAYBACK_ONESHOT:
            stop();
            break;
            
        case PLAYBACK_LOOP:
        case PLAYBACK_ONELOOP:
            NextFrame_ = FirstFrame_;
            break;
            
        case PLAYBACK_PINGPONG:
            if (RepeatCount_ > 1)
            {
                stop();
                return;
            }
        case PLAYBACK_PINGPONG_LOOP:
            math::Swap(FirstFrame_, LastFrame_);
            
            if (NextFrame_ > static_cast<s32>(Frame_))
                NextFrame_ = Frame_ - 1;
            else
                NextFrame_ = Frame_ + 1;
            
            break;
            
        default:
            break;
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
