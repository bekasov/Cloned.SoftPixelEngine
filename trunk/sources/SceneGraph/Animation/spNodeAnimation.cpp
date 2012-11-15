/*
 * Morph target animation file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spNodeAnimation.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


NodeAnimation::NodeAnimation() :
    Animation   (ANIMATION_NODE ),
    Object_     (0              ),
    Spline_     (0              )
{
}
NodeAnimation::~NodeAnimation()
{
    MemoryManager::deleteMemory(Spline_);
}

void NodeAnimation::addKeyframe(const Transformation &Transform, u32 Duration)
{
    Keyframes_.push_back(SNodeKeyframe(Transform, Duration));
    updateSplineTrack(true);
}
void NodeAnimation::insertKeyframe(u32 Index, const Transformation &Transform, u32 Duration)
{
    if (Index < Keyframes_.size())
    {
        Keyframes_.insert(Keyframes_.begin() + Index, SNodeKeyframe(Transform, Duration));
        updateSplineTrack(true);
    }
}
void NodeAnimation::setKeyframe(u32 Index, const Transformation &Transform, u32 Duration)
{
    if (Index < Keyframes_.size())
    {
        Keyframes_[Index] = SNodeKeyframe(Transform, Duration);
        updateSplineTrack(false);
    }
}
void NodeAnimation::removeKeyframe(u32 Index)
{
    if (Index < Keyframes_.size())
    {
        Keyframes_.erase(Keyframes_.begin() + Index);
        updateSplineTrack(true);
    }
}

void NodeAnimation::clearKeyframes()
{
    Keyframes_.clear();
    if (Spline_)
        Spline_->clear();
}

void NodeAnimation::setupManualAnimation(SceneNode* Node)
{
    Object_ = Node;
}

void NodeAnimation::updateAnimation(scene::SceneNode* Node)
{
    if (!Node || !playing())
        return;
    
    Object_ = Node;
    
    /* Get current playback speed */
    f32 Speed = getSpeed();
    
    if (getFrame() < Keyframes_.size())
    {
        const u64 Duration = Keyframes_[getFrame()].Duration;
        Speed *= 1.0f / (60 * Duration / 1000);
    }
    
    /* Update playback process */
    updatePlayback(Speed);
}

u32 NodeAnimation::getKeyframeCount() const
{
    return Keyframes_.size();
}

void NodeAnimation::interpolate(u32 IndexFrom, u32 IndexTo, f32 Interpolation)
{
    if (!Object_)
        return;
    
    const u32 FrameCount = Keyframes_.size();
    
    if (IndexFrom < FrameCount && IndexTo < FrameCount)
    {
        /* Interpolate transformation */
        Transformation Trans;
        Trans.interpolate(
            Keyframes_[IndexFrom].Transform,
            Keyframes_[IndexTo].Transform,
            Interpolation
        );
        
        /* Set new object position */
        if (Spline_)
        {
            if (IndexFrom < IndexTo)
                Object_->setPosition(Spline_->getPolynom(IndexFrom).calc(Interpolation));
            else
                Object_->setPosition(Spline_->getPolynom(IndexTo).calc(1.0f - Interpolation));
        }
        else
            Object_->setPosition(Trans.getPosition());
        
        /* Set new object rotation and scale */
        Object_->setRotationMatrix(Trans.getRotation().getMatrixTransposed());
        Object_->setScale(Trans.getScale());
    }
}

void NodeAnimation::setSplineTranslation(bool Enable)
{
    if (Enable && !Spline_)
    {
        Spline_ = MemoryManager::createMemory<math::Spline3D>("NodeAnimation::Spline3D");
        updateSplineTrack(true);
    }
    else if (!Enable && Spline_)
        MemoryManager::deleteMemory(Spline_);
}
bool NodeAnimation::getSplineTranslation() const
{
    return Spline_ != 0;
}

void NodeAnimation::setSplineExpansion(const f32 Expansion)
{
    if (Spline_)
        Spline_->setExpansion(Expansion);
}
f32 NodeAnimation::getSplineExpansion() const
{
    return Spline_ ? Spline_->getExpansion() : 0.0f;
}

void NodeAnimation::copy(const Animation* Other)
{
    //!TODO!
}


/*
 * ======= Private: =======
 */

void NodeAnimation::updateSplineTrack(bool isRebuild)
{
    if (Spline_ && !Keyframes_.empty())
    {
        /* Build spline animation track */
        std::vector<dim::vector3df> PosList(Keyframes_.size() + 1);
        
        for (u32 i = 0; i < Keyframes_.size(); ++i)
            PosList[i] = Keyframes_[i].Transform.getPosition();
        
        PosList[Keyframes_.size()] = Keyframes_[0].Transform.getPosition();
        
        if (isRebuild)
            Spline_->create(PosList);
        else
            Spline_->update(PosList);
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
