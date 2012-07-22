/*
 * Skeletal animation file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spSkeletalAnimation.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


SkeletalAnimation::SkeletalAnimation() :
    MeshAnimation   (ANIMATION_SKELETAL ),
    Skeleton_       (0                  ),
    MaxKeyframe_    (0                  )
{
}
SkeletalAnimation::~SkeletalAnimation()
{
    clearSkeletons();
}

AnimationSkeleton* SkeletalAnimation::createSkeleton(bool isNewActiveSkeleton)
{
    /* Create new skeleton object */
    AnimationSkeleton* Skeleton = MemoryManager::createMemory<AnimationSkeleton>("Skeleton");
    SkeletonList_.push_back(Skeleton);
    
    /* Set new active skeleton */
    if (isNewActiveSkeleton)
        setActiveSkeleton(Skeleton);
    
    return Skeleton;
}
void SkeletalAnimation::deleteSkeleton(AnimationSkeleton* Skeleton)
{
    if (Skeleton)
    {
        if (Skeleton_ == Skeleton)
            Skeleton_ = 0;
        MemoryManager::removeElement(SkeletonList_, Skeleton, true);
    }
}
void SkeletalAnimation::clearSkeletons()
{
    MemoryManager::deleteList(SkeletonList_);
}

void SkeletalAnimation::addKeyframe(AnimationJoint* Joint, const KeyframeTransformation &Transformation, u32 Frame)
{
    if (!Joint)
        return;
    
    MaxKeyframe_ = math::Max(MaxKeyframe_, Frame);
    
    /* Search for already existing joint-frame sequence */
    foreach (SJointKeyframe &JointFrame, JointKeyframes_)
    {
        if (JointFrame.Joint == Joint)
        {
            JointFrame.Sequence.addKeyframe(Transformation, Frame);
            return;
        }
    }
    
    /* Attach new joint-frame sequence */
    SJointKeyframe JointFrame;
    {
        JointFrame.Joint = Joint;
        JointFrame.Sequence.addKeyframe(Transformation, Frame);
    }
    JointKeyframes_.push_back(JointFrame);
}

void SkeletalAnimation::removeKeyframe(AnimationJoint* Joint, u32 Frame)
{
    if (!Joint)
        return;
    
    for (std::list<SJointKeyframe>::iterator it = JointKeyframes_.begin(); it != JointKeyframes_.end(); ++it)
    {
        if (it->Joint == Joint)
        {
            /* Remove given keyframe */
            it->Sequence.removeKeyframe(Frame);
            
            /* Remove joint-frame sequence if there are no more keyframes */
            if (!it->Sequence.getKeyframeCount())
                JointKeyframes_.erase(it);
            
            break;
        }
    }
}

void SkeletalAnimation::clearKeyframes()
{
    JointKeyframes_.clear();
    MaxKeyframe_ = 0;
}

void SkeletalAnimation::updateAnimation(SceneNode* Node)
{
    /* Get valid mesh object */
    if (!Skeleton_ || !Node || Node->getType() != scene::NODE_MESH || !playing())
        return;
    
    Mesh* Object = static_cast<Mesh*>(Node);
    
    /* Update playback process */
    updatePlayback(getSpeed());
    
    /* Update the vertex transformation if the object is inside a view frustum of any camera */
    if (checkFrustumCulling(Object))
        Skeleton_->transformVertices();
}

u32 SkeletalAnimation::getKeyframeCount() const
{
    return MaxKeyframe_;
}

void SkeletalAnimation::interpolate(u32 IndexFrom, u32 IndexTo, f32 Interpolation)
{
    foreach (SJointKeyframe &JointFrame, JointKeyframes_)
    {
        /* Transform the current joint by the animation state if the joint is enabled */
        if (JointFrame.Joint && JointFrame.Joint->getEnable())
        {
            JointFrame.Sequence.interpolate(
                JointFrame.Joint->getTransformation(),
                IndexFrom, IndexTo, Interpolation
            );
        }
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
