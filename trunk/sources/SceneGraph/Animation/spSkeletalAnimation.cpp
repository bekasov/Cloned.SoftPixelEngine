/*
 * Skeletal animation file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spSkeletalAnimation.hpp"
#include "Base/spTimer.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


SkeletalAnimation::SkeletalAnimation() :
    MeshAnimation   (ANIMATION_SKELETAL ),
    Skeleton_       (0                  )
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

void SkeletalAnimation::addKeyframe(AnimationJoint* Joint, const Transformation &Transform, u32 Frame)
{
    if (!Joint)
        return;
    
    /* Update keyframe limitation */
    MinFrame_ = math::Min(MinFrame_, Frame);
    MaxFrame_ = math::Max(MaxFrame_, Frame);
    
    /* Search for already existing joint-frame sequence */
    foreach (SJointKeyframe &JointFrame, JointKeyframes_)
    {
        if (JointFrame.Joint == Joint)
        {
            JointFrame.Sequence.addKeyframe(Transform, Frame);
            return;
        }
    }
    
    /* Attach new joint-frame sequence */
    SJointKeyframe JointFrame;
    {
        JointFrame.Joint = Joint;
        JointFrame.Sequence.addKeyframe(Transform, Frame);
    }
    JointKeyframes_.push_back(JointFrame);
}

void SkeletalAnimation::removeKeyframe(AnimationJoint* Joint, u32 Frame)
{
    if (!Joint)
        return;
    
    //!TODO! -> this is not correct!!!
    if (Frame == MaxFrame_)
        --MaxFrame_;
    
    /* Update keyframe limitation */
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
    MaxFrame_ = 0;
}

AnimationJointGroup* SkeletalAnimation::addJointGroup(const io::stringc &Name)
{
    AnimationJointGroup* NewGroup = MemoryManager::createMemory<AnimationJointGroup>("scene::AnimationJointGroup");
    
    NewGroup->setName(Name);
    JointGroups_.push_back(NewGroup);
    
    return NewGroup;
}
void SkeletalAnimation::removeJointGroup(AnimationJointGroup* JointGroup)
{
    MemoryManager::removeElement(JointGroups_, JointGroup, true);
}

void SkeletalAnimation::clearJointGroups()
{
    JointGroups_.clear();
}

void SkeletalAnimation::groupJoint(AnimationJointGroup* Group, AnimationJoint* Joint)
{
    if (Group && Joint)
    {
        foreach (SJointKeyframe &JointFrame, JointKeyframes_)
        {
            if (JointFrame.Joint == Joint)
            {
                Group->JointKeyframesRef_.push_back(&JointFrame);
                break;
            }
        }
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("SkeletalAnimation::groupJoint");
    #endif
}

void SkeletalAnimation::ungroupJoint(AnimationJointGroup* Group, AnimationJoint* Joint)
{
    if (Group && Joint)
    {
        for (std::vector<SJointKeyframe*>::iterator it = Group->JointKeyframesRef_.begin(); it != Group->JointKeyframesRef_.end(); ++it)
        {
            if ((*it)->Joint == Joint)
            {
                Group->JointKeyframesRef_.erase(it);
                break;
            }
        }
    }
    #ifdef SP_DEBUGMODE
    else
        io::Log::debug("SkeletalAnimation::ungroupJoint");
    #endif
}

AnimationJointGroup* SkeletalAnimation::findJointGroup(const io::stringc &Name) const
{
    foreach (AnimationJointGroup* Group, JointGroups_)
    {
        if (Group->getName() == Name)
            return Group;
    }
    return 0;
}

void SkeletalAnimation::updateAnimation(SceneNode* Node)
{
    /* Get valid mesh object */
    if (!Skeleton_ || !Node || Node->getType() != scene::NODE_MESH || !playing())
        return;
    
    Mesh* Object = static_cast<Mesh*>(Node);
    
    /* Update playback process */
    updatePlayback(getSpeed() * io::Timer::getGlobalSpeed());
    
    /* Update the vertex transformation if the object is inside a view frustum of any camera */
    if (checkFrustumCulling(Object))
        Skeleton_->transformVertices();
}

void SkeletalAnimation::updateAnimationGroups(SceneNode* Node)
{
    /* Get valid mesh object */
    if (!Skeleton_ || !Node || Node->getType() != scene::NODE_MESH)
        return;
    
    Mesh* Object = static_cast<Mesh*>(Node);
    
    /* Update playback process for all joint groups */
    const f32 AnimSpeed = getSpeed() * io::Timer::getGlobalSpeed();
    
    foreach (AnimationJointGroup* Group, JointGroups_)
        updateJointGroup(Group, AnimSpeed);
    
    /* Update the vertex transformation if the object is inside a view frustum of any camera */
    if (checkFrustumCulling(Object))
        Skeleton_->transformVertices();
}

u32 SkeletalAnimation::getKeyframeCount() const
{
    return MaxFrame_ - MinFrame_ + 1; //todo -> this is not the count of keyframes
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


/*
 * ======= Private: =======
 */

void SkeletalAnimation::updateJointGroup(AnimationJointGroup* Group, f32 AnimSpeed)
{
    /* Update joint transformations */
    foreach (SJointKeyframe* JointFrame, Group->JointKeyframesRef_)
    {
        /* Transform the current joint by the animation state if the joint is enabled */
        if (JointFrame->Joint && JointFrame->Joint->getEnable())
        {
            JointFrame->Sequence.interpolate(
                JointFrame->Joint->getTransformation(),
                Group->Playback_.getFrame(), Group->Playback_.getNextFrame(), Group->Playback_.getInterpolation()
            );
        }
    }
    
    /* Update joint group playback */
    Group->Playback_.update(Group->Playback_.getSpeed() * AnimSpeed);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
