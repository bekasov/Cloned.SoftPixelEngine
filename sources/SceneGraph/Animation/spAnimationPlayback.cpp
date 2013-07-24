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
    BaseObject      (                   ),
    Mode_           (PLAYBACK_ONESHOT   ),
    HasStarted_     (false              ),
    IsPlaying_      (false              ),
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
        
        frameCallback(false);
        
        /* Check if one-loop animation has already done */
        if (Mode_ == PLAYBACK_ONELOOP && Frame_ == FirstFrame_)
        {
            stopAutoAnim();
            return false;
        }
        
        /* Check if current frame arrived the last frame */
        if (Frame_ == LastFrame_)
            checkAnimationEnding();
    }
    
    return playing();
}

bool AnimationPlayback::play(const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame)
{
    /* Don't play animation if first- and last frame are equal or there are no keyframes */
    if (FirstFrame == LastFrame)
        return false;
    
    /* Setup animation playback */
    Mode_           = Mode;
    
    HasStarted_     = true;
    IsPlaying_      = true;
    
    FirstFrame_     = FirstFrame;
    LastFrame_      = LastFrame;
    
    Frame_          = FirstFrame_;
    RepeatCount_    = 0;
    
    /* Setup initial next frame */
    if (LastFrame_ >= FirstFrame_)
        NextFrame_ = FirstFrame_ + 1;
    else
        NextFrame_ = FirstFrame_ - 1;
    
    frameCallback(false);
    
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

void AnimationPlayback::pause(bool IsPaused)
{
    if (HasStarted_)
        IsPlaying_ = !IsPaused;
}

void AnimationPlayback::stop(bool IsReset)
{
    /* Reset state */
    IsPlaying_      = false;
    HasStarted_     = false;
    RepeatCount_    = 0;
    
    if (IsReset)
    {
        /* Reset indices and interpolation */
        Frame_          = 0;
        NextFrame_      = 0;
        Interpolation_  = 0.0f;
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
    
    frameCallback(true);
}

void AnimationPlayback::setFirstFrame(u32 Index)
{
    /* Set new first frame */
    if (FirstFrame_ == Index)
        return;
    
    FirstFrame_ = Index;
    
    /* Check if playback must be stoped */
    if (playing() && static_cast<s32>(FirstFrame_) > math::Min(static_cast<s32>(Frame_), NextFrame_))
        stop();
}

void AnimationPlayback::setLastFrame(u32 Index)
{
    /* Set new last frame */
    if (LastFrame_ == Index)
        return;
    
    LastFrame_ = Index;
    
    /* Check if playback must be stoped */
    if (playing() && static_cast<s32>(LastFrame_) < math::Max(static_cast<s32>(Frame_), NextFrame_))
        stop();
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

SAnimSequence AnimationPlayback::getSequence(u32 SeqId) const
{
    std::map<u32, SAnimSequence>::const_iterator it = Sequences_.find(SeqId);
    
    if (it != Sequences_.end())
        return it->second;
    
    return SAnimSequence();
}

bool AnimationPlayback::playingSeq(u32 SeqId) const
{
    if (!playing())
        return false;
    
    std::map<u32, SAnimSequence>::const_iterator it = Sequences_.find(SeqId);
    
    if (it != Sequences_.end())
    {
        const u32 First = it->second.FirstFrame;
        const u32 Last = it->second.LastFrame;
        
        return (Last > First)
            ? ( Frame_ >= First && Frame_ <= Last )
            : ( Frame_ >= Last && Frame_ <= First );
    }
    
    return false;
}

bool AnimationPlayback::interpolateRange(u32 &FirstFrame, u32 &LastFrame, f32 &Interpolation)
{
    if (FirstFrame >= LastFrame)
        return false;
    
    /* Get frame range */
    math::clamp(Interpolation, 0.0f, 1.0f);
    
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
            stopAutoAnim();
            break;
            
        case PLAYBACK_LOOP:
        case PLAYBACK_ONELOOP:
            NextFrame_ = FirstFrame_;
            break;
            
        case PLAYBACK_PINGPONG:
            if (RepeatCount_ > 1)
            {
                stopAutoAnim();
                return;
            }
        case PLAYBACK_PINGPONG_LOOP:
            std::swap(FirstFrame_, LastFrame_);
            
            if (NextFrame_ > static_cast<s32>(Frame_))
                NextFrame_ = Frame_ - 1;
            else
                NextFrame_ = Frame_ + 1;
            
            break;
            
        default:
            break;
    }
}


/*
 * SAnimQueue structure
 */

SAnimQueue::SAnimQueue()
{
}
SAnimQueue::~SAnimQueue()
{
}

void SAnimQueue::addFrame(u32 Frame, f32 Speed)
{
    Queue.push_back(SFrame(Frame, Speed));
}
void SAnimQueue::removeEntry(u32 QueueEntryIndex)
{
    if (QueueEntryIndex < Queue.size())
        Queue.erase(Queue.begin() + QueueEntryIndex);
}
void SAnimQueue::clear()
{
    Queue.clear();
}


} // /namespace scene

} // /namespace sp



// ================================================================================
