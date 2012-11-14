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
#include "SceneGraph/Animation/spAnimationJointGroup.hpp"
#include "SceneGraph/Animation/spAnimationBaseStructures.hpp"
#include "SceneGraph/Animation/spKeyframeSequence.hpp"

#include <vector>


namespace sp
{
namespace scene
{


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
        
        //! Clears the animation keyframes for all animation joints.
        void clearKeyframes();
        
        /**
        Adds a new joint group. Joint groups are not stored in the skeleton because it
        deals more with animation playback than skeleton construction.
        \param[in] Name Specifies the group name.
        \return Pointer to the new AnimationJointGroup object.
        \see groupJoint
        \note Joint groups can be used to animate several bone groups parallel,
        e.g. let a character's legs be animated while the arms are transformed procedural.
        */
        AnimationJointGroup* addJointGroup(const io::stringc &Name = "");
        /**
        Removes the specified joint-group object.
        \see addJointGroup
        */
        void removeJointGroup(AnimationJointGroup* JointGroup);
        
        //! Clears (or rather removes) all joint groups.
        void clearJointGroups();
        
        /**
        Inserts the specified joint to the specified joint-group.
        \param[in] Group Specifies the pointer to the joint-group object.
        This must be created previously with the "addJointGroup" function.
        \param[in] Joint Specifies the pointer to the joint object.
        Joints can only be created with the skeleton.
        \see addJointGroup
        \see AnimationSkeleton
        */
        void groupJoint(AnimationJointGroup* Group, AnimationJoint* Joint);
        /**
        Ungroups a joint from a joint-group.
        \see groupJoint
        */
        void ungroupJoint(AnimationJointGroup* Group, AnimationJoint* Joint);
        
        //! Returns a pointer to the first joint-group with the specified name.
        AnimationJointGroup* findJointGroup(const io::stringc &Name) const;
        
        /**
        Updates the skeletal animation. If the animation is playing all keyframes will be performed.
        \param[in] Object This must be a Mesh scene node. Otherwise the function will do nothing.
        */
        virtual void updateAnimation(SceneNode* Node);
        /**
        Updates all joint group animations for this the skeletal animation.
        \param[in] Object This must be a Mesh scene node. Otherwise the function will do nothing.
        \see addJointGroup
        */
        virtual void updateAnimationGroups(SceneNode* Node);
        
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
        
        /**
        Returns the active skeleton.
        \see setActiveSkeleton
        */
        AnimationSkeleton* getActiveSkeleton() const
        {
            return Skeleton_;
        }
        
    private:
        
        /* === Functions === */
        
        void updateJointGroup(AnimationJointGroup* Group, f32 AnimSpeed);
        
        /* === Members === */
        
        AnimationSkeleton* Skeleton_;                   //!< Active skeleton.
        std::list<AnimationSkeleton*> SkeletonList_;    //!< Skeleton objects created by this animation.
        
        std::list<SJointKeyframe> JointKeyframes_;      //!< Joint keyframes.
        
        /**
        Joint groups for grouped-animation. This is not stored in the skeleton because it
        deals more with animation playback than skeleton construction.
        */
        std::vector<AnimationJointGroup*> JointGroups_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
