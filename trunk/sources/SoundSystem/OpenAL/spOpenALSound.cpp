/*
 * OpenAL sound file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenAL/spOpenALSound.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "Base/spMemoryManagement.hpp"
#include "Base/spInputOutputLog.hpp"
#include "SoundSystem/OpenAL/spOpenALSoundDevice.hpp"
#include "SoundSystem/OpenAL/spOpenALBufferObject.hpp"
#include "SoundSystem/OpenAL/spOpenALSoundEffect.hpp"
#include "SoundSystem/OpenAL/spOpenALExtensions.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern audio::SoundDevice* __spSoundDevice;

namespace audio
{


OpenALSound::OpenALSound() :
    Sound           (   ),
    BufferObject_   (0  )
{
}
OpenALSound::~OpenALSound()
{
    close();
}

bool OpenALSound::reload(const io::stringc &Filename, u8 BufferCount)
{
    if (!checkFile(Filename))
        return false;
    
    /* Close previous loaded sound */
    close();
    
    /* Create new OpenAL buffer object */
    BufferObject_ = static_cast<OpenALSoundDevice*>(__spSoundDevice)->createSoundBuffer(Filename);
    
    if (!BufferObject_)
    {
        io::Log::lowerTab();
        return false;
    }
    
    /* Create new OpenAL source objects */
    SourceObjects_  .resize(BufferCount);
    SourceObjectIDs_.resize(BufferCount);
    
    for (u8 i = 0; i < BufferCount; ++i)
    {
        SourceObjects_[i] = MemoryManager::createMemory<ALSourceObject>("OpenALSound::ALSourceObject");
        SourceObjects_[i]->setupBuffer(BufferObject_);
        SourceObjectIDs_[i] = SourceObjects_[i]->getID();
    }
    
    /* Initialize source object iterator */
    CurSourceObject_ = SourceObjects_.begin();
    
    Filename_ = Filename;
    io::Log::lowerTab();
    
    return true;
}

void OpenALSound::close()
{
    static_cast<OpenALSoundDevice*>(__spSoundDevice)->dropSoundBuffer(BufferObject_);
    
    MemoryManager::deleteList(SourceObjects_);
    SourceObjectIDs_.clear();
}

void OpenALSound::play()
{
    Sound::play();
    
    if (!SourceObjects_.empty())
    {
        ++CurSourceObject_;
        if (CurSourceObject_ == SourceObjects_.end())
            CurSourceObject_ = SourceObjects_.begin();
        
        const ALuint ALSource = getSourceID();
        
        alSourcePlay(ALSource);
    }
}

void OpenALSound::pause(bool Paused)
{
    Sound::pause(Paused);
    
    if (!SourceObjectIDs_.empty())
    {
        if (Paused)
            alSourcePausev(SourceObjectIDs_.size(), &SourceObjectIDs_[0]);
        else
            alSourcePlayv(SourceObjectIDs_.size(), &SourceObjectIDs_[0]);
    }
}

void OpenALSound::stop()
{
    Sound::stop();
    
    if (!SourceObjectIDs_.empty())
        alSourceRewindv(SourceObjectIDs_.size(), &SourceObjectIDs_[0]);
}

void OpenALSound::setSeek(f32 Seek)
{
    if (!SourceObjects_.empty())
        alSourcef(getSourceID(), AL_SEC_OFFSET, Seek * getLength());
}

f32 OpenALSound::getSeek() const
{
    if (!SourceObjects_.empty())
    {
        f32 Seek = 0.0f;
        alGetSourcef(getSourceID(), AL_SEC_OFFSET, &Seek);
        return Seek / getLength();
    }
    
    return 0.0f;
}

void OpenALSound::setVolume(f32 Volume)
{
    Sound::setVolume(Volume);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcef(ALSource, AL_GAIN, Volume);
}

void OpenALSound::setSpeed(f32 Speed)
{
    Sound::setSpeed(Speed);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcef(ALSource, AL_PITCH, Speed);
}

void OpenALSound::setBalance(f32 Balance)
{
    //todo
}

void OpenALSound::setLoop(bool Enable)
{
    Sound::setLoop(Enable);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcei(ALSource, AL_LOOPING, isLoop_);
}

f32 OpenALSound::getLength() const
{
    return 1.0f; //todo
}

bool OpenALSound::valid() const
{
    return BufferObject_ && alIsBuffer(BufferObject_->getID());
}

void OpenALSound::setPosition(const dim::vector3df &Position)
{
    Sound::setPosition(Position);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcefv(ALSource, AL_POSITION, &Position.X);
}
void OpenALSound::setVelocity(const dim::vector3df &Veclotiy)
{
    Sound::setVelocity(Veclotiy);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcefv(ALSource, AL_VELOCITY, &Veclotiy.X);
}

void OpenALSound::setVolumetric(bool isVolumetric)
{
    Sound::setVolumetric(isVolumetric);
    
    //...
}
void OpenALSound::setVolumetricRadius(f32 Radius)
{
    Sound::setVolumetricRadius(Radius);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcef(ALSource, AL_ROLLOFF_FACTOR, 100.0f / Radius);
}

void OpenALSound::setSoundEffect(SoundEffect* SoundEffectObject)
{
    if (Effect_ != SoundEffectObject)
    {
        Sound::setSoundEffect(SoundEffectObject);
        
        if (Effect_)
        {
            ALuint ALEffectSlot = static_cast<OpenALSoundEffect*>(SoundEffectObject)->ALEffectSlot_;
            
            foreach (ALuint ALSource, SourceObjectIDs_)
                alSource3i(ALSource, AL_AUXILIARY_SEND_FILTER, ALEffectSlot, 0, AL_FILTER_NULL);
        }
        else
        {
            foreach (ALuint ALSource, SourceObjectIDs_)
                alSource3i(ALSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
        }
    }
}

bool OpenALSound::playing() const
{
    return !SourceObjects_.empty() && (*CurSourceObject_)->getState() == AL_PLAYING;
}
bool OpenALSound::finish() const
{
    return !playing(); //todo
}


/*
 * ======= Private: =======
 */


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
