/*
 * Animation playback file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spAnimationPlayback.hpp"
#include "Base/spMath.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


AnimationPlayback::AnimationPlayback() :
    Mode_           (PLAYBACK_ONESHOT   ),
    hasStarted_     (false              ),
    isPlaying_      (false              ),
    Frame_          (0                  ),
    NextFrame_      (0                  ),
    Interpolation_  (0.0f               ),
    FirstFrame_     (0                  ),
    LastFrame_      (0                  ),
    Speed_          (1.0f               ),
    RepeatCount_    (0                  )
{
}
AnimationPlayback::~AnimationPlayback()
{
}

bool AnimationPlayback::update(f32 Speed)
{
    if (!playing())
        return false;
    
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
            return false;
        }
        
        /* Check if current frame arrived the last frame */
        if (Frame_ == LastFrame_)
            checkAnimationEnding();
    }
    
    return true;
}

bool AnimationPlayback::play(const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame)
{
    /* Don't play animation if first- and last frame are equal or there are no keyframes */
    if (FirstFrame == LastFrame)
        return false;
    
    /* Setup animation playback */
    Mode_           = Mode;
    
    hasStarted_     = true;
    isPlaying_      = true;
    
    Frame_          = FirstFrame_;
    RepeatCount_    = 0;
    
    /* Setup initial next frame */
    if (LastFrame_ >= FirstFrame_)
        NextFrame_ = FirstFrame_ + 1;
    else
        NextFrame_ = FirstFrame_ - 1;
    
    return true;
}

bool AnimationPlayback::play(u32 SeqId)
{
    std::map<u32, SAnimSequence>::iterator it = Sequences_.find(SeqId);
    
    if (it != Sequences_.end())
    {
        setSpeed(it->second.Speed);
        return play(it->second.Mode, it->second.FirstFrame, it->second.LastFrame);
    }
    
    return false;
}

void AnimationPlayback::pause(bool isPaused)
{
    if (hasStarted_)
        isPlaying_ = !isPaused;
}

void AnimationPlayback::stop()
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

void AnimationPlayback::setFrame(u32 Index)
{
    /* Setup current frame */
    if (Frame_ == Index)
        return;
    
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
    else
        NextFrame_ = Frame_ + 1;
}

bool AnimationPlayback::addSequence(
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

bool AnimationPlayback::removeSequence(u32 SeqId)
{
    std::map<u32, SAnimSequence>::iterator it = Sequences_.find(SeqId);
    
    if (it != Sequences_.end())
    {
        Sequences_.erase(it);
        return true;
    }
    
    return false;
}

void AnimationPlayback::clearSequences()
{
    Sequences_.clear();
}

bool AnimationPlayback::interpolateRange(u32 &FirstFrame, u32 &LastFrame, f32 &Interpolation)
{
    if (FirstFrame >= LastFrame)
        return false;
    
    /* Get frame range */
    math::Clamp(Interpolation, 0.0f, 1.0f);
    
    const u32 From  = FirstFrame + static_cast<u32>(Interpolation * (LastFrame - FirstFrame));
    const u32 To    = From + 1;
    
    /* Get final interpolation factor */
    Interpolation *= (LastFrame - FirstFrame);
    Interpolation -= (From - FirstFrame);
    
    /* Get final frame indices */
    FirstFrame  = From;
    LastFrame   = To;
    
    return true;
}


/*
 * ======= Private: =======
 */

void AnimationPlayback::checkAnimationEnding()
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
