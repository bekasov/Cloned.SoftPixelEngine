/*
 * Skeletal animation header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SKELETAL_ANIMATION_H__
#define __SP_SKELETAL_ANIMATION_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/Animation/spMeshAnimation.hpp"
#include "SceneGraph/Animation/spAnimationSkeleton.hpp"
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
Skeletal animation class. An skeletal animation consists primary of a skeleton (AnimationSkeleton object) and
joint keyframes. This skeleton consits of joints (AnimationJoint objects) and theses joints have their own
vertex weights information i.e. data which describes how a vertex will be influenced by each joint.
\ingroup group_animation
*/
class SP_EXPORT SkeletalAnimation : public MeshAnimation
{
    
    public:
        
        SkeletalAnimation();
        virtual ~SkeletalAnimation();
        
        /* === Functions === */
        
        //! Creates a new skeleton and sets by default this to the new active one.
        AnimationSkeleton* createSkeleton(bool isNewActiveSkeleton = true);
        
        //! Deletes the specified skeleton if it was created by this animation.
        void deleteSkeleton(AnimationSkeleton* Skeleton);
        
        //! Deletes all skeletons.
        void clearSkeletons();
        
        /**
        Adds a new keyframe for the specified joint.
        \param Joint: Specifies the joint which is to be affected by that keyframe.
        \param Transform: Specifies the transformation for this keyframe.
        \param Frame: Specifies the frame index. For each joint a frame index may be used only once.
        */
        void addKeyframe(AnimationJoint* Joint, const Transformation &Transform, u32 Frame);
        
        /**
        Removes the keyframe for the specified joint.
        \param Joint: Specifies the joint for which the keyframe is to be removed.
        \param Frame: Specifies the frame index at which position the keyframe is to be removed.
        */
        void removeKeyframe(AnimationJoint* Joint, u32 Frame);
        
        void clearKeyframes();
        
        /**
        Updates the skeletal animation. If the animation is playing all keyframes will be performed.
        \param Object: This must be a Mesh scene node. Otherwise the function will do nothing.
        */
        virtual void updateAnimation(SceneNode* Node);
        
        virtual u32 getKeyframeCount() const;
        
        virtual void interpolate(u32 IndexFrom, u32 IndexTo, f32 Interpolation);
        
        /* === Inline functions === */
        
        /**
        Sets the new skeleton for this skeletal animation. You can create a skeleton only for this
        animation (use SkeletalAnimation::createSkeleton). And you can use a skeleton from another
        animation but then you need to remove the skeleton yourself before this other animation
        will be deleted!
        */
        inline void setActiveSkeleton(AnimationSkeleton* Skeleton)
        {
            Skeleton_ = Skeleton;
        }
        
        //! Returns the 
        AnimationSkeleton* getActiveSkeleton() const
        {
            return Skeleton_;
        }
        
    private:
        
        /* === Members === */
        
        AnimationSkeleton* Skeleton_;                   //!< Active skeleton.
        std::list<AnimationSkeleton*> SkeletonList_;    //!< Skeleton objects created by this animation.
        
        std::list<SJointKeyframe> JointKeyframes_;        //!< Joint keyframes.
        
        u32 MaxKeyframe_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
