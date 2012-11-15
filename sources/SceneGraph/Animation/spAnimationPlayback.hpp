/*
 * Animation playback header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ANIMATION_PLAYBACK_H__
#define __SP_ANIMATION_PLAYBACK_H__


#include "Base/spStandard.hpp"

#include <map>


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


//! Animation sequence structure. This is used to store a fixed animation sequence for easy access.
struct SAnimSequence
{
    SAnimSequence() :
        Mode        (PLAYBACK_ONESHOT   ),
        FirstFrame  (0                  ),
        LastFrame   (0                  ),
        Speed       (1.0f               )
    {
    }
    ~SAnimSequence()
    {
    }
    
    /* Members */
    EAnimPlaybackModes Mode;
    u32 FirstFrame;
    u32 LastFrame;
    f32 Speed;
};


/**
Animation playback class.
\see Animation
\ingroup group_animation
*/
class SP_EXPORT AnimationPlayback
{
    
    public:
        
        AnimationPlayback();
        ~AnimationPlayback();
        
        /* === Functions === */
        
        /**
        Updates the playback process with the specified interpolation speed.
        \return True if the animation is currently playing.
        */
        bool update(f32 Speed);
        
        /**
        Plays the animation.
        \param Mode: Specifies the animation mode.
        \param FirstFrame: Specifies the first animation frame.
        \param LastFrame: Specifies the last animation frame.
        \return True if the animation could be played. Otherwise the first- and last frame are equal.
        */
        bool play(const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame);
        
        /**
        Plays the given animation sequence.
        \return True if the specified sequence ID has previously been registerd with "addSequence".
        \see addSequence
        */
        bool play(u32 SeqId);
        
        //! Pauses or resumes the animation.
        void pause(bool isPaused = true);
        
        //! Stops the animation.
        void stop();
        
        //! Sets the new frame index. Should not be used while the animation is playing.
        void setFrame(u32 Index);
        
        /**
        Adds a new sequence. This just a memory for a limited animation sequence.
        Use this to store several sequences like "character moving", "weapon reloaded" etc.
        \return True if the new sequence could be added. Otherwise the given ID was already reserved.
        \see play
        */
        bool addSequence(
            u32 SeqId, const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame, f32 Speed = 1.0f
        );
        
        //! Removes the specified sequence.
        bool removeSequence(u32 SeqId);
        
        //! Clears all sequences.
        void clearSequences();
        
        /* === Static functions === */
        
        /**
        Makes an interpolation over the given range. In this case the first frame must be smaller then the last frame!
        \param[in,out] FirstFrame Specifies the first frame in the range.
        \param[in,out] LastFrame Specifies the last frame in the range.
        \param[in,out] Interpolation Specifies the interpolation factor. Must be in the range [0.0 .. 1.0].
        \return True if the given range is valid.
        */
        static bool interpolateRange(u32 &FirstFrame, u32 &LastFrame, f32 &Interpolation);
        
        /* === Inline functions === */
        
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
        //! Returns the index for the next frame.
        inline u32 getNextFrame() const
        {
            return NextFrame_;
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
        inline EAnimPlaybackModes getMode() const
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
        
    private:
        
        /* === Functions === */
        
        void checkAnimationEnding();
        
        /* === Members === */
        
        EAnimPlaybackModes Mode_;
        
        bool hasStarted_;
        bool isPlaying_;
        
        u32 Frame_;                         //!< Current frame index.
        s32 NextFrame_;                     //!< This can be temporarily negative, thus is it a signed ineger.
        f32 Interpolation_;                 //!< Current frame interpolation factor [0.0 .. 1.0].
        
        u32 FirstFrame_, LastFrame_;        //!< Animation frame range.
        
        f32 Speed_;                         //!< Playback speed (by default 1.0).
        u32 RepeatCount_;                   //!< Repetition counter to support ping-pong animations.
        
        std::map<u32, SAnimSequence> Sequences_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
