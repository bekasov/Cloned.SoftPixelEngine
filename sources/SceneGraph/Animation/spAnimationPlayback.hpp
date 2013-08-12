/*
 * Animation playback header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ANIMATION_PLAYBACK_H__
#define __SP_ANIMATION_PLAYBACK_H__


#include "Base/spStandard.hpp"
#include "Base/spBaseObject.hpp"

#include <map>
#include <boost/function.hpp>


namespace sp
{
namespace scene
{


static const u32 ANIM_LAST_FRAME = ~0;


class AnimationPlayback;

/**
The playback frame callback can be used to determine when the frame index of an individual animation playback has been changed.
\param[in,out] Playback Specifies the animation playback object whose frame index has changed.
\param[in] isSetManual Specifies whether the frame index change was caused by a "setFrame" function call or not.
If false the frame index changed was caused during the common animation playback.
*/
typedef boost::function<void (AnimationPlayback &Playback, bool isSetManual)> PlaybackFrameCallback;


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
Animation playback queue structure. This is used to store custom playback sequences.
For default playback sequences the next frame is always the current frame plus one (NextFrame := CurrentFrame + 1).
But for playback queues the next frame can be specified individually.
Also the playback speed can be manipulated for each frame.
*/
struct SP_EXPORT SAnimQueue
{
    SAnimQueue();
    ~SAnimQueue();
    
    /* === Functions === */
    
    /**
    Adds a new frame to the queue.
    \param[in] Frame Specifies the frame index which is to be added.
    \param[in] Speed Specifies the speed multiplier. This will be multiplied
    by the animation playback speed. By default 1.0.
    */
    void addFrame(u32 Frame, f32 Speed = 1.0f);
    /**
    Removes the specified queue entry.
    \param[in] QueueEntryIndex Specifies the index for the queue's entry which is to be removed.
    \note "QueueEntryIndex" does not specify a frame index but a queue's entry index.
    */
    void removeEntry(u32 QueueEntryIndex);
    /**
    Clears all frame entries. This is equivalent to the following code:
    \code
    MyAnimQueue.Queue.clear();
    \endcode
    */
    void clear();
    
    //! Internal frame structure.
    struct SFrame
    {
        SFrame() :
            Frame(0     ),
            Speed(1.0f  )
        {
        }
        SFrame(u32 InitFrame, f32 InitSpeed = 1.0f) :
            Frame(InitFrame),
            Speed(InitSpeed)
        {
        }
        ~SFrame()
        {
        }
        
        /* Members */
        u32 Frame; //!< Frame index which is to be interpolated.
        f32 Speed; //!< Speed multiplier. This will be multiplied by the animation playback speed.
    };
    
    /* === Members === */
    std::vector<SFrame> Queue;
};


/**
Animation playback class. This class only stores information about the playback, i.e. frame indices,
playback- speed and mode etc. But no information about the underlying animated object.
\see Animation
\since Version 3.2
\ingroup group_animation
*/
class SP_EXPORT AnimationPlayback : public BaseObject
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
        \param[in] Mode Specifies the animation mode.
        \param[in] FirstFrame Specifies the first animation frame.
        \param[in] LastFrame Specifies the last animation frame.
        \return True if the animation could be played. Otherwise the first- and last frame are equal.
        */
        bool play(const EAnimPlaybackModes Mode, u32 FirstFrame, u32 LastFrame);
        
        /**
        Plays the given animation sequence.
        \return True if the specified sequence ID has previously been registerd with "addSequence".
        \see addSequence
        */
        bool play(u32 SeqId);
        
        //! Pauses or resumes the playback process.
        void pause(bool IsPaused = true);
        
        /**
        Stops the playback process.
        \param[in] IsReset Specifies whether frame and interpolation are to be reseted. By default false.
        */
        void stop(bool IsReset = false);
        
        //! Sets the new frame index. If the new index is out of the playback range, the animation will stop.
        void setFrame(u32 Index);
        
        //! Sets the new first frame index. If the current frame index is out of the playback range, the animation will stop.
        void setFirstFrame(u32 Index);
        //! Sets the new last frame index. If the current frame index is out of the playback range, the animation will stop.
        void setLastFrame(u32 Index);
        
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
        
        /**
        Returns the specified playback sequence. If the specified sequence could not be found,
        a default (and invalid) sequence will be returned. 'Invalid' means that the first- and last
        frame indices are 0.
        */
        SAnimSequence getSequence(u32 SeqId) const;
        /**
        Returns true if the specified (and previously with the "addSequence" function added)
        palyback sequence is currently being played.
        */
        bool playingSeq(u32 SeqId) const;
        
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
            return IsPlaying_;
        }
        
        /**
        Returns true if the animation is currently playing and the current frame index is in
        the range [MinFrame, MaxFrame). This is equivalent to the following code:
        \code
        return Playback.playing() && Playback.getFrame() >= MinFrame && Playback.getFrame() < MaxFrame;
        \endcode
        */
        inline bool playing(u32 MinFrame, u32 MaxFrame) const
        {
            return IsPlaying_ && Frame_ >= MinFrame && Frame_ < MaxFrame;
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
        
        //! Returns the first frame. This will be set when starting playback or with the "setFirstFrame" function.
        inline u32 getFirstFrame() const
        {
            return FirstFrame_;
        }
        //! Returns the last frame. This will be set when starting playback or with the "setLastFrame" function.
        inline u32 getLastFrame() const
        {
            return LastFrame_;
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
        
        //! Sets the new playback mode.
        inline void setMode(const EAnimPlaybackModes Mode)
        {
            Mode_ = Mode;
        }
        //! Returns the current playback mode.
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
        
        /**
        Sets the playback frame callback.
        \see PlaybackFrameCallback
        */
        inline void setFrameCallback(const PlaybackFrameCallback &FrameCallback)
        {
            FrameCallback_ = FrameCallback;
        }
        //! Returns the playback frame callback. By default null.
        inline PlaybackFrameCallback getFrameCallback() const
        {
            return FrameCallback_;
        }
        
    private:
        
        /* === Functions === */
        
        void checkAnimationEnding();
        
        /* === Inline functions === */
        
        inline void frameCallback(bool isSetManual)
        {
            if (FrameCallback_)
                FrameCallback_(*this, isSetManual);
        }
        
        inline void stopAutoAnim()
        {
            Interpolation_ = 0.0f;
            stop();
        }
        
        /* === Members === */
        
        EAnimPlaybackModes Mode_;
        
        bool HasStarted_;                   //!< True if animation has started (also when it's paused).
        bool IsPlaying_;                    //!< True while animation is playing and not paused.
        
        u32 Frame_;                         //!< Current frame index.
        s32 NextFrame_;                     //!< This can be temporarily negative, thus is it a signed integer.
        f32 Interpolation_;                 //!< Current frame interpolation factor [0.0 .. 1.0].
        
        u32 FirstFrame_, LastFrame_;        //!< Animation frame range.
        
        f32 Speed_;                         //!< Playback speed (by default 1.0).
        u32 RepeatCount_;                   //!< Repetition counter to support ping-pong animations.
        
        std::map<u32, SAnimSequence> Sequences_;
        //std::map<u32, SAnimQueue> Queues_;
        
        PlaybackFrameCallback FrameCallback_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
