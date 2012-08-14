/*
 * Sound interface file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/spSound.hpp"
#include "Base/spInputOutputLog.hpp"
#include "Base/spInputOutputFileSystem.hpp"


namespace sp
{
namespace audio
{


Sound::Sound() :
    BufferCount_    (0      ),
    Radius_         (100.0f ),
    Volume_         (1.0f   ),
    Balance_        (0.0f   ),
    Speed_          (1.0f   ),
    Seek_           (0.0f   ),
    isPaused_       (false  ),
    isLoop_         (false  ),
    isPlaying_      (false  ),
    isVolumetric_   (false  ),
    isRecording_    (false  ),
    hasSeekChanged_ (false  ),
    Effect_         (0      )
{
}
Sound::~Sound()
{
}

void Sound::play()
{
    isPlaying_  = true;
    isPaused_   = false;
}
void Sound::pause(bool Paused)
{
    if (isPlaying_)
        isPaused_ = Paused;
}
void Sound::stop()
{
    isPlaying_  = false;
    isPaused_   = false;
}

void Sound::setSeek(f32 Seek)
{
    Seek_ = Seek;
}
f32 Sound::getSeek() const
{
    return Seek_;
}

void Sound::setVolume(f32 Volume)
{
    Volume_ = Volume;
}
void Sound::setSpeed(f32 Speed)
{
    Speed_ = Speed;
}
void Sound::setBalance(f32 Balance)
{
    Balance_ = Balance;
}

void Sound::setLoop(bool Enable)
{
    isLoop_ = Enable;
}

void Sound::setSoundEffect(SoundEffect* SoundEffectObject)
{
    Effect_ = SoundEffectObject;
}

bool Sound::saveRecord(const io::stringc &Filename)
{
    return false; // do nothing
}
void Sound::beginRecording()
{
    // do nothing
}
void Sound::endRecording()
{
    // do nothing
}

void Sound::setVolumetric(bool isVolumetric)
{
    isVolumetric_ = isVolumetric;
}
void Sound::setVolumetricRadius(f32 Radius)
{
    Radius_ = Radius;
}

void Sound::setPosition(const dim::vector3df &Position)
{
    Position_ = Position;
}
void Sound::setVelocity(const dim::vector3df &Veclotiy)
{
    Veclotiy_ = Veclotiy;
}

bool Sound::playing() const
{
    return isPlaying_;
}
bool Sound::finish() const
{
    return getSeek() >= 1.0f;
}


/*
 * ======= Private: =======
 */

bool Sound::checkFile(const io::stringc &Filename) const
{
    /* Information message */
    io::Log::message("Load sound: \"" + Filename + "\"");
    io::Log::upperTab();
    
    /* Check if the file does exist */
    if (!io::FileSystem().findFile(Filename))
    {
        io::Log::error("Could not find sound file");
        io::Log::lowerTab();
        return false;
    }
    
    return true;
}


} // /namespace audio

} // /namespace sp



// ================================================================================
