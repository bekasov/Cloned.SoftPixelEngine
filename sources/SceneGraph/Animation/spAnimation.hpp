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

#include <list>
#include <vector>


namespace sp
{
namespace scene
{


static const u32 ANIM_LAST_FRAME = ~0;


//! Animation playback modes.
enum EAnimPlaybackModes
{
    PLAYBACK_ONESHOT,       //!< From first- to last frame.
    PLAYBACK_ONELOOP,       //!< From first- to last frame and back to first frame.
    PLAYBACK_LOOP,          //!< From first- to last frame forever.
    PLAYBACK_PINGPONG,      //!< From first- to last frame and backwards.
    PLAYBACK_PINGPONG_LOOP, //!< From first- to last frame and backwards forever.
};

//! Types of animation.
enum EAnimationTypes
{
    ANIMATION_NODE,         //!< Node animation. An scene node will be transformed (position, rotation and scale).
    ANIMATION_MORPHTARGET,  //!< Morph-target animation. Vertices of a mesh object will be transformed (vertex coordinate and normal).
    ANIMATION_SKELETAL,     //!< Skeletal animation. Consists of a skeleton (AnimationSkeleton object) which holds all joints (AnimationJoint objects).
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
        */
        virtual void play(const EAnimPlaybackModes Mode, u32 FirstFrame = 0, u32 LastFrame = ANIM_LAST_FRAME);
        
        //! Pauses or resumes the animation.
        virtual void pause(bool isPaused = true);
        
        //! Stops the animation.
        virtual void stop();
        
        //! Clears all keyframes.
        virtual void clearKeyframes() = 0;
        
        //! Sets the new frame index. Should not be used while the animation is playing.
        virtual void setFrame(u32 Index);
        
        //! Setups a manual animation process. Call this function before interpolating manually between two frames.
        virtual void setupManualAnimation(SceneNode* Node);
        
        //! Updates the animation process if currently it's playing.
        virtual void updateAnimation(SceneNode* Node) = 0;
        
        //! Returns the count of keyframes.
        virtual u32 getKeyframeCount() const = 0;
        
        //! Makes an interpolation between the two given frames.
        virtual void interpolate(u32 IndexFrom, u32 IndexTo, f32 Interpolation) = 0;
        
        //! Makes an interpolation over the given sequence. In this case the first frame must be smaller then the last frame!
        virtual void interpolateSequence(u32 FirstFrame, u32 LastFrame, f32 Interpolation);
        
        /**
        Adds the specified scene node to the animatable object list. All these objects
        will be animated when this animation is being played using SceneGraph::updateAnimations.
        */
        virtual void addSceneNode(SceneNode* Object);
        
        //! Removes the specified scene node from the animatable object list.
        virtual void removeSceneNode(SceneNode* Object);
        
        //! Removes the animatable object list.
        virtual void clearSceneNodes();
        
        /* === Inline functions === */
        
        //! Returns the type of animation: Node-, MorphTarget- or SkeletalAnimation.
        inline EAnimationTypes getType() const
        {
            return Type_;
        }
        
        //! Sets the animations name.
        inline void setName(const io::stringc &Name)
        {
            Name_ = Name;
        }
        inline io::stringc getName() const
        {
            return Name_;
        }
        
        //! Returns true if the animation is currently playing.
        inline bool playing() const
        {
            return isPlaying_;
        }
        
        //! Returns the current frame index.
        inline u32 getFrame() const
        {
            return Frame_;
        }
        
        //! Sets the new frame interpolation.
        inline void setInterpolation(f32 Interpolation)
        {
            Interpolation_ = Interpolation;
        }
        //! Returns the current frame interpolation.
        inline f32 getInterpolation() const
        {
            return Interpolation_;
        }
        
        //! Returns the current animation playback mode.
        inline EAnimPlaybackModes getPlaybackMode() const
        {
            return Mode_;
        }
        
        //! Sets the new playback speed (by default 1.0).
        inline void setSpeed(f32 Speed)
        {
            Speed_ = Speed;
        }
        inline f32 getSpeed() const
        {
            return Speed_;
        }
        
        //! Returns the animatable scene node list.
        inline const std::list<SceneNode*>& getSceneNodeList() const
        {
            return SceneNodes_;
        }
        
    protected:
        
        Animation(const EAnimationTypes Type);
        
        /* === Functions === */
        
        //! Updates the playback process with the specified interpolation speed.
        virtual void updatePlayback(f32 Speed);
        
        //! Returns the valid keyframe index.
        u32 getValidFrame(u32 Index) const;
        
    private:
        
        /* === Functions === */
        
        void checkAnimationEnding();
        
        /* === Members === */
        
        EAnimationTypes Type_;
        EAnimPlaybackModes Mode_;
        
        io::stringc Name_;
        
        bool hasStarted_;
        bool isPlaying_;
        
        u32 Frame_;                         //!< Current frame index.
        s32 NextFrame_;                     //!< This can be temporarily negative, thus is it a signed ineger.
        f32 Interpolation_;                 //!< Current frame interpolation factor [0.0 .. 1.0].
        
        u32 FirstFrame_, LastFrame_;        //!< Animation frame range.
        u32 MinFrame_, MaxFrame_;
        
        f32 Speed_;                         //!< Playback speed (by default 1.0).
        u32 RepeatCount_;                   //!< Repetition counter to support ping-pong animations.
        
        std::list<SceneNode*> SceneNodes_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
