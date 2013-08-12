/*
 * Texture animation file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spTextureAnimation.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "Base/spTimer.hpp"


namespace sp
{
namespace scene
{


TextureAnimation::TextureAnimation() :
    Playing_(false  ),
    Paused_ (false  ),
    Index_  (0      ),
    Time_   (0      ),
    Speed_  (1.0f   )
{
}
TextureAnimation::~TextureAnimation()
{
}

bool TextureAnimation::play(f32 Speed)
{
    if (Frames_.size() >= 2)
    {
        /* Initialize animation playback */
        Playing_            = true;
        Paused_             = false;
        Index_              = 0;
        Time_               = io::Timer::millisecs();
        PauseTimeOffset_    = 0;
        Speed_              = Speed;
        return true;
    }
    return false;
}

void TextureAnimation::pause(bool isPaused)
{
    if (Paused_ != isPaused)
    {
        /* Setup pause state */
        Paused_ = isPaused;
        
        /* Update time offset */
        if (Paused_)
            PauseTimeOffset_ = io::Timer::millisecs();
        else
            Time_ += (io::Timer::millisecs() - PauseTimeOffset_);
    }
}

void TextureAnimation::stop()
{
    /* Reset animation playback (but hold speed setting) */
    Playing_            = false;
    Paused_             = false;
    Index_              = 0;
    Time_               = 0;
    PauseTimeOffset_    = 0;
}

size_t TextureAnimation::addFrame(const STextureAnimFrame &Frame)
{
    if (Frame.Tex && Frame.Duration > 0)
    {
        Frames_.push_back(Frame);
        return Frames_.size();
    }
    return 0;
}

bool TextureAnimation::removeFrame(size_t Index)
{
    if (Index < Frames_.size())
    {
        Frames_.erase(Frames_.begin() + Index);
        return true;
    }
    return false;
}

size_t TextureAnimation::removeFrames(video::Texture* Tex)
{
    if (Tex)
    {
        size_t Num = 0;
        
        for (std::vector<STextureAnimFrame>::iterator it = Frames_.begin(); it != Frames_.end();)
        {
            if (it->Tex == Tex)
            {
                it = Frames_.erase(it);
                ++Num;
            }
            else
                ++it;
        }
        
        return Num;
    }
    
    return 0;
}

void TextureAnimation::clearFrames()
{
    Frames_.clear();
}

void TextureAnimation::update(video::Texture* OutputTexture)
{
    /* Update animation playback */
    if (Frames_.size() >= 2 && Playing_)
    {
        if (!Paused_)
        {
            if (io::Timer::millisecs() > Time_ + Frames_[Index_].Duration)
            {
                /* Increment frame index and start at beginning when it's at the end */
                ++Index_;
                
                if (Index_ >= Frames_.size())
                    Index_ = 0;
            }
        }
        
        /* Setup new animation frame for specified texture */
        if (OutputTexture)
            OutputTexture->setReference(Frames_[Index_].Tex);
    }
    else
    {
        /* Disable texture referencing when animation has stoped */
        if (OutputTexture)
            OutputTexture->setReference(0);
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
