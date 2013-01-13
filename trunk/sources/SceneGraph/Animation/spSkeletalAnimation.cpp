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
    clearJointGroups();
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

void SkeletalAnimation::addKeyframe(AnimationJoint* Joint, u32 Frame, const Transformation &Transform)
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
            JointFrame.Sequence.addKeyframe(Frame, Transform);
            return;
        }
    }
    
    /* Attach new joint-frame sequence */
    SJointKeyframe JointFrame;
    {
        JointFrame.Joint = Joint;
        JointFrame.Sequence.addKeyframe(Frame, Transform);
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
    
    if (Name.size())
        JointGroupsMap_[Name.str()] = NewGroup;
    
    return NewGroup;
}
void SkeletalAnimation::removeJointGroup(AnimationJointGroup* JointGroup)
{
    if (JointGroup)
    {
        /* Remove joint-group from hash-map */
        if (JointGroup->getName().size())
        {
            std::map<std::string, AnimationJointGroup*>::iterator it = JointGroupsMap_.find(JointGroup->getName().str());
            if (it != JointGroupsMap_.end())
                JointGroupsMap_.erase(it);
        }
        
        /* Delete joint-group object */
        MemoryManager::removeElement(JointGroups_, JointGroup, true);
    }
}

void SkeletalAnimation::clearJointGroups()
{
    MemoryManager::deleteList(JointGroups_);
    JointGroupsMap_.clear();
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
    std::map<std::string, AnimationJointGroup*>::const_iterator it = JointGroupsMap_.find(Name.str());
    
    if (it != JointGroupsMap_.end())
        return it->second;
    
    return 0;
}

bool SkeletalAnimation::playGroup(const io::stringc &Name, const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame)
{
    AnimationJointGroup* Group = findJointGroup(Name);
    if (Group)
        return Group->getPlayback().play(Mode, FirstFrame, LastFrame);
    return false;
}
bool SkeletalAnimation::playGroup(const io::stringc &Name, u32 SeqId)
{
    AnimationJointGroup* Group = findJointGroup(Name);
    if (Group)
        return Group->getPlayback().play(SeqId);
    return false;
}
void SkeletalAnimation::pauseGroup(const io::stringc &Name, bool isPaused)
{
    AnimationJointGroup* Group = findJointGroup(Name);
    if (Group)
        Group->getPlayback().pause(isPaused);
}
void SkeletalAnimation::stopGroup(const io::stringc &Name)
{
    AnimationJointGroup* Group = findJointGroup(Name);
    if (Group)
        Group->getPlayback().stop();
}

void SkeletalAnimation::poseGroup(const io::stringc &Name, u32 Frame, f32 Interpolation)
{
    AnimationJointGroup* Group = findJointGroup(Name);
    if (Group)
    {
        Group->getPlayback().stop();
        Group->getPlayback().setFrame(Frame);
        Group->getPlayback().setInterpolation(Interpolation);
    }
}

void SkeletalAnimation::updateAnimation(SceneNode* Node)
{
    /* Check if animation must be updated */
    const bool isGroupAnim = (!JointGroups_.empty() && !(Flags_ & ANIMFLAG_NO_GROUPING));
    
    if ( !Skeleton_ || !Node || Node->getType() != scene::NODE_MESH || ( !isGroupAnim && !playing() ) )
        return;
    
    /* Update playback process */
    const f32 AnimSpeed = getSpeed() * io::Timer::getGlobalSpeed();
    
    if (isGroupAnim)
    {
        foreach (AnimationJointGroup* Group, JointGroups_)
            updateJointGroup(Group, AnimSpeed);
    }
    else
        updatePlayback(AnimSpeed);
    
    if (!(Flags_ & ANIMFLAG_NO_TRANSFORMATION))
    {
        /* Update the vertex transformation if the object is inside a view frustum of any camera */
        scene::Mesh* MeshObj = static_cast<Mesh*>(Node);
        
        if (checkFrustumCulling(MeshObj))
            Skeleton_->transformVertices(MeshObj);
    }
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

void SkeletalAnimation::copy(const Animation* Other)
{
    if (!Other || Other->getType() != ANIMATION_SKELETAL)
        return;
    
    /* Copy animation base */
    copyBase(Other);
    
    /* Copy skeletal-animation specific content */
    const SkeletalAnimation* AnimTemplate = static_cast<const SkeletalAnimation*>(Other);
    
    JointKeyframes_ = AnimTemplate->JointKeyframes_;
    
    MemoryManager::deleteList(JointGroups_);
    JointGroups_.resize(AnimTemplate->JointGroups_.size());
    
    for (u32 i = 0, c = JointGroups_.size(); i < c; ++i)
    {
        /* Create new joint-group */
        AnimationJointGroup* JointGroup = JointGroups_[i] = MemoryManager::createMemory<AnimationJointGroup>("scene::AnimationJointGroup");
        
        /*
         * Make a 'raw' copy of joint group and use existing instance of
         * keyframe references (AnimationJointGroup::JointKeyframesRef_)
         */
        *JointGroup = *AnimTemplate->JointGroups_[i];
        
        /* Insert joint-group to hash-map */
        if (JointGroup->getName().size())
            JointGroupsMap_[JointGroup->getName().str()] = JointGroup;
    }
    
    /* Set active skeleton to existing instance */
    setActiveSkeleton(AnimTemplate->getActiveSkeleton());
}


/*
 * ======= Private: =======
 */

void SkeletalAnimation::updateJointGroup(AnimationJointGroup* Group, f32 AnimSpeed)
{
    /* Update joint group playback */
    Group->Playback_.update(Group->Playback_.getSpeed() * AnimSpeed);
    
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
}


} // /namespace scene

} // /namespace sp



// ================================================================================
