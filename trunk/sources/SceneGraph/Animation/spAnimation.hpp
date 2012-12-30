/*
 * Animation interface header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ANIMATION_H__
#define __SP_ANIMATION_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneNode.hpp"
#include "SceneGraph/Animation/spAnimationPlayback.hpp"

#include <list>


namespace sp
{
namespace scene
{


//! Types of animation classes.
enum EAnimationTypes
{
    ANIMATION_NODE,         //!< Node animation. An scene node will be transformed (position, rotation and scale).
    ANIMATION_MORPHTARGET,  //!< Morph-target animation. Vertices of a mesh object will be transformed (vertex coordinate and normal).
    ANIMATION_SKELETAL,     //!< Skeletal animation. Consists of a skeleton (AnimationSkeleton object) which holds all joints (AnimationJoint objects).
};

//! Animation flags.
enum EAnimationFlags
{
    ANIMFLAG_NO_GROUPING        = 0x01, //!< Disables the automatic group-animation for skeletal-animations.
    ANIMFLAG_NO_TRANSFORMATION  = 0x02, //!< Disables the vertex transformation for skeletal-animations.
};


/**
Animation base class. This is the base class for animation objects. It has the fundamental functions like playing and updating the animation.
\see SkeletalAnimation
\see NodeAnimation
\see MorphTargetAnimation
\ingroup group_animation
*/
class SP_EXPORT Animation
{
    
    public:
        
        virtual ~Animation();
        
        /* === Functions === */
        
        /**
        Plays the animation.
        \param Mode: Specifies the animation mode.
        \param FirstFrame: Specifies the first animation frame.
        \param LastFrame: Specifies the last animation frame. This can also be smaller than the first frame.
        By default ANIM_IGNORE_FRAME which means that all frames will be played.
        \return True if the animation could be played. Otherwise the first- and last frame are equal or
        there are only less than 2 keyframes.
        */
        virtual bool play(const EAnimPlaybackModes Mode, u32 FirstFrame = 0, u32 LastFrame = ANIM_LAST_FRAME);
        
        //! Pauses or resumes the animation.
        virtual void pause(bool IsPaused = true);
        
        //! Stops the animation.
        virtual void stop(bool IsReset = false);
        
        //! Clears all keyframes.
        virtual void clearKeyframes() = 0;
        
        //! Sets the new frame index. Should not be used while the animation is playing.
        virtual void setFrame(u32 Index);
        
        //! Updates the animation process if currently it's playing.
        virtual void updateAnimation(SceneNode* Node) = 0;
        
        //! Returns the count of keyframes.
        virtual u32 getKeyframeCount() const = 0;
        
        //! Makes an interpolation between the two given frames.
        virtual void interpolate(u32 IndexFrom, u32 IndexTo, f32 Interpolation) = 0;
        
        //! Makes an interpolation over the given sequence. In this case the first frame must be smaller then the last frame!
        virtual bool interpolateRange(u32 FirstFrame, u32 LastFrame, f32 Interpolation);
        
        /**
        Adds the specified scene node to the animatable object list. All these objects
        will be animated when this animation is being played using SceneGraph::updateAnimations.
        */
        virtual void addSceneNode(SceneNode* Object);
        
        //! Removes the specified scene node from the animatable object list.
        virtual void removeSceneNode(SceneNode* Object);
        
        //! Removes the animatable object list.
        virtual void clearSceneNodes();
        
        /**
        Copies the whole animation.
        \return Pointer to the new animation object.
        \note You have to delete this pointer by yourself!
        */
        virtual void copy(const Animation* Other) = 0;
        
        /* === Inline functions === */
        
        //! Returns the type of animation: Node-, MorphTarget- or SkeletalAnimation.
        inline EAnimationTypes getType() const
        {
            return Type_;
        }
        
        //! Sets the animation name.
        inline void setName(const io::stringc &Name)
        {
            Name_ = Name;
        }
        //! Returns the animation name.
        inline io::stringc getName() const
        {
            return Name_;
        }
        
        /**
        Sets the animation flags.
        \see EAnimationFlags
        */
        inline void setFlags(s32 Flags)
        {
            Flags_ = Flags;
        }
        //! Returns the animation flags. By default 0.
        inline s32 getFlags() const
        {
            return Flags_;
        }
        
        //! Returns true if the animation is currently playing.
        inline bool playing() const
        {
            return Playback_.playing();
        }
        
        //! Returns the current frame index.
        inline u32 getFrame() const
        {
            return Playback_.getFrame();
        }
        
        //! Returns the minimal keyframe index.
        inline u32 getMinFrame() const
        {
            return MinFrame_;
        }
        //! Returns the maximal keyframe index.
        inline u32 getMaxFrame() const
        {
            return MaxFrame_;
        }
        
        //! Sets the new frame interpolation.
        inline void setInterpolation(f32 Interpolation)
        {
            Playback_.setInterpolation(Interpolation);
        }
        //! Returns the current frame interpolation.
        inline f32 getInterpolation() const
        {
            return Playback_.getInterpolation();
        }
        
        //! Returns the current animation playback mode.
        inline EAnimPlaybackModes getPlaybackMode() const
        {
            return Playback_.getMode();
        }
        
        //! Sets the new playback speed (by default 1.0).
        inline void setSpeed(f32 Speed)
        {
            Playback_.setSpeed(Speed);
        }
        inline f32 getSpeed() const
        {
            return Playback_.getSpeed();
        }
        
        //! Returns a constant reference to the playback state object.
        inline const AnimationPlayback& getPlayback() const
        {
            return Playback_;
        }
        //! Returns a reference to the playback state object.
        inline AnimationPlayback& getPlayback()
        {
            return Playback_;
        }
        
        //! Returns the animatable scene node list.
        inline const std::list<SceneNode*>& getSceneNodeList() const
        {
            return SceneNodes_;
        }
        
    protected:
        
        /* === Functions === */
        
        Animation(const EAnimationTypes Type);
        
        //! Updates the playback process with the specified interpolation speed.
        virtual void updatePlayback(f32 Speed);
        
        //! Returns the valid keyframe index.
        u32 getValidFrame(u32 Index) const;
        
        void copyBase(const Animation* Other);
        
        /* === Members === */
        
        u32 MinFrame_, MaxFrame_;
        
        AnimationPlayback Playback_;
        
        s32 Flags_;
        
    private:
        
        /* === Members === */
        
        EAnimationTypes Type_;
        io::stringc Name_;
        
        std::list<SceneNode*> SceneNodes_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
