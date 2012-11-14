/*
 * Animation joint group header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ANIMATION_JOINT_GROUP_H__
#define __SP_ANIMATION_JOINT_GROUP_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Animation/spAnimationPlayback.hpp"
#include "SceneGraph/Animation/spAnimationJoint.hpp"
#include "SceneGraph/Animation/spAnimationBaseStructures.hpp"
#include "SceneGraph/Animation/spKeyframeSequence.hpp"

#include <vector>


namespace sp
{
namespace scene
{


//! Joint keyframe structure.
struct SJointKeyframe
{
    SJointKeyframe() : Joint(0)
    {
    }
    ~SJointKeyframe()
    {
    }
    
    /* Members */
    AnimationJoint* Joint;      //!< Reference to the joint.
    KeyframeSequence Sequence;  //!< The keyframe sequence for the joint.
};


/**
Animation joint groups can only be created with an SkeletalAnimation class instance.
\see SkeletalAnimation
\ingroup group_animation
*/
class SP_EXPORT AnimationJointGroup
{
    
    public:
        
        AnimationJointGroup();
        ~AnimationJointGroup();
        
        /* Functions */
        
        //! Clears the joint list and stops the playback. This list can only be filled with an SkeletalAnimation class instance.
        void clearJoints();
        
        /* Inline functions */
        
        inline void setName(const io::stringc &Name)
        {
            Name_ = Name;
        }
        inline io::stringc getName() const
        {
            return Name_;
        }
        
        inline const AnimationPlayback& getPlayback() const
        {
            return Playback_;
        }
        inline AnimationPlayback& getPlayback()
        {
            return Playback_;
        }
        
    private:
        
        friend class SkeletalAnimation;
        
        /* Members */
        
        io::stringc Name_;
        AnimationPlayback Playback_;
        std::vector<SJointKeyframe*> JointKeyframesRef_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
