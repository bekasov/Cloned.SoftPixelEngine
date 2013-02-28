/*
 * OpenAL sound device header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDDEVICE_OPENAL_H__
#define __SP_AUDIO_SOUNDDEVICE_OPENAL_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "SoundSystem/spSoundDevice.hpp"
#include "SoundSystem/OpenAL/spOpenALSound.hpp"

#include <AL/al.h>
#include <AL/alc.h>
#include <map>


namespace sp
{
namespace audio
{


struct SAudioBuffer;

class OpenALSoundDevice : public SoundDevice
{
        
    public:
            
        OpenALSoundDevice();
        ~OpenALSoundDevice();
        
        /* Functions */
        
        io::stringc getInterface() const;
        
        Sound* createSound();
        SoundEffect* createSoundEffect();
        
        void updateSounds();
        
        void setListenerPosition(const dim::vector3df &Position);
        void setListenerVelocity(const dim::vector3df &Velocity);
        void setListenerOrientation(const dim::matrix4f &Orientation);
        
        void setListenerSpeed(f32 Speed);
        
        void setEffectSlot(SoundEffect* Sfx);
        
    private:
        
        friend class OpenALSound;
        
        typedef std::map<std::string, ALBufferObject*> AudioBufferType;
        
        /* === Macros === */
        
        static const f32 DEFAULT_SOUND_SPEED;
        
        /* === Functions === */
        
        bool openALDevice();
        void closeALDevice();
        
        bool loadExtensions();
        
        ALBufferObject* createSoundBuffer(const io::stringc &Filename);
        void dropSoundBuffer(ALBufferObject* &BufferObj);
        
        /* === Members === */
        
        ALCdevice* ALDevice_;
        ALCcontext* ALContext_;
        
        ALuint ALEffectSlot_;
        
        bool HasExtensions_;
        
        static AudioBufferType AudioBufferMap_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
