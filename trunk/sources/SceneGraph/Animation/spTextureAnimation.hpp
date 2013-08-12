/*
 * Texture animation header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TEXTURE_ANIMATION_H__
#define __SP_TEXTURE_ANIMATION_H__


#include "Base/spStandard.hpp"

#include <vector>


namespace sp
{
namespace video
{
    class Texture;
}
namespace scene
{


/**
Texture animation frame structure.
\since Version 3.3
*/
struct STextureAnimFrame
{
    STextureAnimFrame() :
        Tex     (0),
        Duration(0)
    {
    }
    STextureAnimFrame(video::Texture* FrameTex, u64 FrameDuration) :
        Tex     (FrameTex       ),
        Duration(FrameDuration  )
    {
    }
    STextureAnimFrame(const STextureAnimFrame &Other) :
        Tex     (Other.Tex      ),
        Duration(Other.Duration )
    {
    }
    ~STextureAnimFrame()
    {
    }
    
    /* Members */
    video::Texture* Tex;
    u64 Duration;
};


/**
Texture animation class. Interpolation between two texture frames is NOT supported.
\note For this animation class no "AnimationPlayback" object is used, i.e. only a
simple foreward animation is used here for simplicity. For a more detailed texture
animation you have to use an "AnimationPlayback" object and set each texture frame
by yourself using the "Texture::setReference" function.
\see Texture::setReference
\since Version 3.3
\ingroup group_animation
*/
class SP_EXPORT TextureAnimation
{
    
    public:
        
        TextureAnimation();
        ~TextureAnimation();
        
        /* === Functions === */
        
        /**
        Starts playing the animation.
        \param[in] Speed Specifies the playback speed factor. By default 1.0.
        \return True if the animation has started. Otherwise the animation has less than two frames.
        */
        bool play(f32 Speed = 1.0f);
        /**
        Pauses or resumes the animation playback.
        \param[in] isPaused Specifies whether the playback is to be pasued or resumed. By default paused.
        */
        void pause(bool isPaused = true);
        //! Stops the animation playback. It also resets the frame index to zero but keeps the speed factor setting.
        void stop();
        
        /**
        Adds a new frame to the animation.
        \param[in] Frame Specifies the new frame which is to be added.
        \return Count of frames after adding this frame or zero is the frame could not be added.
        \see STextureAnimFrame
        */
        size_t addFrame(const STextureAnimFrame &Frame);
        
        /**
        Removes the specified frame from the animation.
        \param[in] Index Specifies the index to the frame which is to be removed.
        \return True if the specified frame could be removed. Otherwise the index is out of range.
        */
        bool removeFrame(size_t Index);
        /**
        Removes all frames from the animation which have a pointer to the specified texture object.
        \param[in] Tex Pointer to the texture for finding all frames which are to be removed.
        \return Number of removed frames.
        */
        size_t removeFrames(video::Texture* Tex);
        
        //! Clears all animation frames.
        void clearFrames();
        
        void update(video::Texture* OutputTexture);
        
        /* === Inline functions === */
        
        /**
        Returns true if the animation is currently being played.
        \see play
        \see stop
        */
        inline bool playing() const
        {
            return Playing_;
        }
        /**
        Returns true if the animation is currently being paused.
        \see pause
        */
        inline bool paused() const
        {
            return Paused_;
        }
        
        //! Returns the current frame index. This is always in the range [0 .. Number-of-Fames).
        inline u32 getIndex() const
        {
            return Index_;
        }
        
        //! Sets the new playback speed factor. By default 1.0.
        inline void setSpeed(f32 Speed)
        {
            Speed_ = Speed;
        }
        //! Returns the playback speed factor. By default 1.0.
        inline f32 getSpeed() const
        {
            return Speed_;
        }
        
        /**
        Adds a new frame to the animation.
        \param[in] Tex Pointer to the texture which is to be added to the animation.
        If null this function call has no effect and the return value is zero.
        \param[in] Duration Specifies the duration (in milliseconds) this frame is visible during animation playback.
        The playback speed can also be configured using the speed factor.
        \return Count of frames after adding this frame or zero is the frame could not be added.
        \see addFrame(const STextureAnimFrame&)
        \see setSpeed
        */
        inline size_t addFrame(video::Texture* Tex, u64 Duration)
        {
            return addFrame(STextureAnimFrame(Tex, Duration));
        }
        
        //! Returns the whole animation frame list.
        inline const std::vector<STextureAnimFrame>& getFrameList() const
        {
            return Frames_;
        }
        
    private:
        
        /* === Members === */
        
        bool Playing_;                          //!< Specifies whether the animation is currently being played or stoped.
        bool Paused_;                           //!< Specifies whether the animation is currently paused or not.
        u32 Index_;                             //!< Current frame index.
        u64 Time_;                              //!< Time when the last frame has changed.
        u64 PauseTimeOffset_;                   //!< Time offset for pausing animation.
        f32 Speed_;                             //!< Animation playback speed factor. By default 1.0.
        
        std::vector<STextureAnimFrame> Frames_;   //!< Animation frame textures.
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
