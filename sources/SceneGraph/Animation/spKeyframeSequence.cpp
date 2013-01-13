/*
 * Keyframe sequence file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spKeyframeSequence.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


KeyframeSequence::KeyframeSequence() :
    MinFrame_       (0     ),
    MaxFrame_       (0     ),
    Modified_       (false ),
    UpdateImmediate_(false  )
{
}
KeyframeSequence::~KeyframeSequence()
{
}

#if 1 //!!!

void KeyframeSequence::addKeyPosition(u32 Frame, const dim::vector3df &Position)
{
    insertKey(ConstructKeysPos_, SKeyPos(Frame, Position));
    markAsModified();
}

void KeyframeSequence::addKeyRotation(u32 Frame, const dim::quaternion &Rotation)
{
    insertKey(ConstructKeysRot_, SKeyRot(Frame, Rotation));
    markAsModified();
}

void KeyframeSequence::addKeyScale(u32 Frame, const dim::vector3df &Scale)
{
    insertKey(ConstructKeysScl_, SKeyScl(Frame, Scale));
    markAsModified();
}

void KeyframeSequence::addKeyframe(u32 Frame, s32 _deb_ToBeremoved_, const Transformation &Transform, s32 Flags)
{
    if (Flags)
    {
        if (Flags & KEYFRAME_POSITION)
            insertKey(ConstructKeysPos_, SKeyPos(Frame, Transform.getPosition()));
        if (Flags & KEYFRAME_ROTATION)
            insertKey(ConstructKeysRot_, SKeyRot(Frame, Transform.getRotation()));
        if (Flags & KEYFRAME_SCALE)
            insertKey(ConstructKeysScl_, SKeyScl(Frame, Transform.getScale()));
        markAsModified();
    }
}

bool KeyframeSequence::removeKeyframe(u32 Frame, s32 _deb_ToBeremoved_, s32 Flags)
{
    if (!Flags)
        return false;
    
    /* Remove keys */
    bool HasAnyRemoved = false;
    
    if (Flags & KEYFRAME_POSITION)
    {
        if (removeKey(ConstructKeysPos_, Frame))
            HasAnyRemoved = true;
    }
    if (Flags & KEYFRAME_ROTATION)
    {
        if (removeKey(ConstructKeysRot_, Frame))
            HasAnyRemoved = true;
    }
    if (Flags & KEYFRAME_SCALE)
    {
        if (removeKey(ConstructKeysScl_, Frame))
            HasAnyRemoved = true;
    }
    
    /* Update frame range */
    if (Flags & KEYFRAME_ALL)
    {
        if (Frame == MinFrame_)
        {
            MinFrame_ = 999999;
            
            filterMinFrame(ConstructKeysPos_);
            filterMinFrame(ConstructKeysRot_);
            filterMinFrame(ConstructKeysScl_);
        }
        if (Frame == MaxFrame_)
        {
            MaxFrame_ = 0;
            
            filterMaxFrame(ConstructKeysPos_);
            filterMaxFrame(ConstructKeysRot_);
            filterMaxFrame(ConstructKeysScl_);
        }
    }
    else
    {
        if (Frame == MinFrame_ || Frame == MaxFrame_)
            updateFrameRangeComplete();
    }
    
    /* Update modification */
    if (HasAnyRemoved)
    {
        markAsModified();
        return true;
    }
    
    return false;
}

bool KeyframeSequence::updateSequence()
{
    if (!Modified_)
        return false;
    
    Modified_ = false;
    
    /* Check if there have been any keyframes added */
    if (ConstructKeysPos_.empty() && ConstructKeysRot_.empty() && ConstructKeysScl_.empty())
    {
        Keyframes_.clear();
        return true;
    }
    
    /* Construct final transformations */
    Keyframes_.resize(MaxFrame_ + 1);
    
    u32 Frame = 0;
    u32 FromIndex[3] = { 0 }, ToIndex[3] = { 0 };
    
    dim::vector3df Pos, PosFrom, PosTo;
    dim::quaternion Rot, RotFrom, RotTo;
    dim::vector3df Scl, SclFrom(1.0f), SclTo(1.0f);
    
    std::vector<SKeyPos>::iterator itPos = getFirstInterpVectors(ConstructKeysPos_, PosFrom, PosTo, FromIndex[0], ToIndex[0]);
    std::vector<SKeyRot>::iterator itRot = getFirstInterpVectors(ConstructKeysRot_, RotFrom, RotTo, FromIndex[1], ToIndex[1]);
    std::vector<SKeyScl>::iterator itScl = getFirstInterpVectors(ConstructKeysScl_, SclFrom, SclTo, FromIndex[2], ToIndex[2]);
    
    const bool NoneEmptyList[3] =
    {
        ConstructKeysPos_.empty(),
        ConstructKeysRot_.empty(),
        ConstructKeysScl_.empty()
    };
    
    foreach (Transformation &Trans, Keyframes_)
    {
        /* Calculate position */
        if (NoneEmptyList[0])
        {
            const f32 Interp = static_cast<f32>(Frame - FromIndex[0]) / (ToIndex[0] - FromIndex[0]);
            
            math::Lerp(Pos, PosFrom, PosTo, Interp);
            Trans.setPosition(Pos);
            
            getNextInterpIterator(ConstructKeysPos_, itPos, Frame, PosFrom, PosTo, FromIndex[0], ToIndex[0]);
        }
        
        /* Calculate rotation */
        if (NoneEmptyList[1])
        {
            const f32 Interp = static_cast<f32>(Frame - FromIndex[1]) / (ToIndex[1] - FromIndex[1]);
            
            Rot.slerp(RotFrom, RotTo, Interp);
            Trans.setRotation(Rot);
            
            getNextInterpIterator(ConstructKeysRot_, itRot, Frame, RotFrom, RotTo, FromIndex[1], ToIndex[1]);
        }
        
        /* Calculate scaling */
        if (NoneEmptyList[2])
        {
            const f32 Interp = static_cast<f32>(Frame - FromIndex[2]) / (ToIndex[2] - FromIndex[2]);
            
            math::Lerp(Scl, SclFrom, SclTo, Interp);
            Trans.setScale(Scl);
            
            getNextInterpIterator(ConstructKeysScl_, itScl, Frame, SclFrom, SclTo, FromIndex[2], ToIndex[2]);
        }
        
        ++Frame;
    }
    
    return true;
}

void KeyframeSequence::setUpdateImmediate(bool Enable)
{
    if (UpdateImmediate_ != Enable)
    {
        UpdateImmediate_ = Enable;
        if (UpdateImmediate_ && Modified_)
            updateSequence();
    }
}

#endif

void KeyframeSequence::addKeyframe(u32 Frame, const Transformation &Transform)
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

#if 1 //!!!

void KeyframeSequence::updateFrameRangeComplete()
{
    if (!ConstructKeysPos_.empty() || !ConstructKeysRot_.empty() || !ConstructKeysScl_.empty())
    {
        MinFrame_ = 999999;
        MaxFrame_ = 0;
        
        updateFrameRange(ConstructKeysPos_);
        updateFrameRange(ConstructKeysRot_);
        updateFrameRange(ConstructKeysScl_);
    }
    else
        MinFrame_ = MaxFrame_ = 0;
}

void KeyframeSequence::markAsModified()
{
    Modified_ = true;
    if (UpdateImmediate_)
        updateSequence();
}

#endif


} // /namespace scene

} // /namespace sp



// ================================================================================
