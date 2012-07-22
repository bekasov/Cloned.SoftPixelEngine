/*
 * XAudio2 sound file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/XAudio2/spXAudio2Sound.hpp"

#if defined(SP_COMPILE_WITH_XAUDIO2)


#include "SoundSystem/XAudio2/spXAudio2SoundDevice.hpp"


namespace sp
{
namespace audio
{


XAudio2Sound::XAudio2Sound() :
    Sound       (   ),
    SoruceVoice_(0  ),
    WaveBuffer_ (0  )
{
}
XAudio2Sound::~XAudio2Sound()
{
    if (SoruceVoice_)
        SoruceVoice_->DestroyVoice();
    MemoryManager::deleteBuffer(WaveBuffer_);
}

bool XAudio2Sound::reload(const io::stringc &Filename, u8 BufferCount)
{
    //...
    
    Filename_ = Filename;
    
    return true;
}

void XAudio2Sound::close()
{
}

void XAudio2Sound::play()
{
    Sound::play();
    
}

void XAudio2Sound::pause(bool Paused)
{
    Sound::pause(Paused);
    
}

void XAudio2Sound::stop()
{
    Sound::stop();
    
}

void XAudio2Sound::setSeek(f32 Seek)
{
    Seek_ = Seek;
    
}

f32 XAudio2Sound::getSeek() const
{
    return 0.0f;
}

void XAudio2Sound::setVolume(f32 Volume)
{
    Volume_ = Volume;
    
}

void XAudio2Sound::setSpeed(f32 Speed)
{
    Speed_ = Speed;
    
}

void XAudio2Sound::setBalance(f32 Balance)
{
    Balance_ = Balance;
    
}

f32 XAudio2Sound::getLength() const
{
    return 0.0f;
}

bool XAudio2Sound::valid() const
{
    return SoruceVoice_ != 0;
}


/*
 * ======= Private: =======
 */



} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
