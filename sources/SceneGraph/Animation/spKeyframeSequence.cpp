/*
 * Keyframe sequence file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spKeyframeSequence.hpp"


namespace sp
{
namespace scene
{


KeyframeSequence::KeyframeSequence()
{
}
KeyframeSequence::~KeyframeSequence()
{
}

void KeyframeSequence::addKeyframe(const Transformation &Transform, u32 Frame)
{
    /* Check if frame index is greater and new elements must be added */
    if (Frame >= Keyframes_.size())
        pushBackKeyframe(Transform, Frame);
    else
        insertKeyframe(Transform, Frame);
}

void KeyframeSequence::removeKeyframe(u32 Frame)
{
    /* Check if last frame is to be removed or an inner frame */
    if (Frame == Keyframes_.size() - 1)
        popBackKeyframe(Frame);
    else if (Frame < Keyframes_.size())
        extractKeyframe(Frame);
}


/*
 * ======= Private: =======
 */

void KeyframeSequence::findRootFrameRange(u32 Frame, u32* LeftFrame, u32* RightFrame)
{
    if (LeftFrame)
    {
        /* Search the next left root keyframe */
        *LeftFrame = Frame;
        
        if (*LeftFrame > 0)
            while (--*LeftFrame && !RootKeyframes_[*LeftFrame]);
    }
    
    if (RightFrame)
    {
        /* Search the next right root keyframe */
        *RightFrame = Frame;
        
        if (*RightFrame < Keyframes_.size() - 1)
            while (++*RightFrame < Keyframes_.size() && !RootKeyframes_[*RightFrame]);
    }
}

void KeyframeSequence::pushBackKeyframe(const Transformation &Transform, u32 Frame)
{
    const u32 FirstNewFrame = Keyframes_.size();
    
    /* Resize the containers */
    Keyframes_      .resize(Frame + 1);
    RootKeyframes_  .resize(Frame + 1);
    
    /* Get last frame transformation */
    Transformation LastTrans;
    
    if (FirstNewFrame > 0)
        LastTrans = Keyframes_[FirstNewFrame - 1];
    
    /* Get interpolation range */
    const s32 FrameCount = 1 + Frame - FirstNewFrame;
    const f32 IntStep = 1.0f / FrameCount;
    
    f32 Factor = 0.0f;
    
    /* Fill the containers */
    for (u32 i = FirstNewFrame; ; ++i)
    {
        if (i < Frame)
        {
            /* Interpolate between the last and the new frame */
            Factor += IntStep;
            Keyframes_[i].interpolate(LastTrans, Transform, Factor);
            
            RootKeyframes_[i] = false;
        }
        else
        {
            /* Store actual new keyframe */
            Keyframes_[i] = Transform;
            
            /* Store this frame as a root keyframe */
            RootKeyframes_[i] = true;
            
            break;
        }
    }
}

void KeyframeSequence::insertKeyframe(const Transformation &Transform, u32 Frame)
{
    /* Temporary memory */
    s32 FrameCount;
    f32 IntStep, Factor;
    
    /* Get left and right root keyframes */
    u32 LeftFrame, RightFrame;
    findRootFrameRange(Frame, &LeftFrame, &RightFrame);
    
    /* Interpolate all keyframes between the left and the current root frame */
    FrameCount = 1 + Frame - LeftFrame;
    
    if (FrameCount >= 3)
    {
        IntStep = 1.0f / FrameCount;
        Factor = IntStep;
        
        for (u32 i = LeftFrame + 1; i < Frame; ++i, Factor += IntStep)
            Keyframes_[i].interpolate(Keyframes_[LeftFrame], Transform, Factor);
    }
    
    /* Interpolate all keyframes between the current and the right root frame */
    FrameCount = 1 + RightFrame - Frame;
    
    if (FrameCount >= 3)
    {
        IntStep = 1.0f / FrameCount;
        Factor = IntStep;
        
        for (u32 i = Frame + 1; i < RightFrame; ++i, Factor += IntStep)
            Keyframes_[i].interpolate(Transform, Keyframes_[RightFrame], Factor);
    }
    
    /* Set the new current keyframe */
    Keyframes_[Frame]       = Transform;
    RootKeyframes_[Frame]   = true;
}

void KeyframeSequence::popBackKeyframe(u32 Frame)
{
    /* Get left and right root keyframes */
    u32 LeftFrame;
    findRootFrameRange(Frame, &LeftFrame, 0);
    
    /* Removes keyframes */
    if (LeftFrame >= Frame - 1)
    {
        /* Only remove the last keyframe */
        Keyframes_.pop_back();
        RootKeyframes_.pop_back();
    }
    else
    {
        Keyframes_.erase(Keyframes_.begin() + LeftFrame + 1, Keyframes_.end());
        RootKeyframes_.erase(RootKeyframes_.begin() + LeftFrame + 1, RootKeyframes_.end());
    }
}

void KeyframeSequence::extractKeyframe(u32 Frame)
{
    /* Get left and right root keyframes */
    u32 LeftFrame, RightFrame;
    findRootFrameRange(Frame, &LeftFrame, &RightFrame);
    
    /* Interpolate between the left and the right keyframes */
    s32 FrameCount = 1 + RightFrame - LeftFrame;
    
    if (FrameCount >= 3)
    {
        f32 IntStep = 1.0f / FrameCount;
        f32 Factor = IntStep;
        
        for (u32 i = LeftFrame + 1; i < RightFrame; ++i, Factor += IntStep)
            Keyframes_[i].interpolate(Keyframes_[LeftFrame], Keyframes_[RightFrame], Factor);
    }
    
    /* Remove root keyframe info */
    RootKeyframes_[Frame] = false;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
