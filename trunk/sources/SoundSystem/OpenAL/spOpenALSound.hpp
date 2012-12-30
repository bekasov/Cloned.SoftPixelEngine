/*
 * OpenAL sound header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUND_OPENAL_H__
#define __SP_AUDIO_SOUND_OPENAL_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "SoundSystem/spSound.hpp"
#include "SoundSystem/OpenAL/spOpenALSourceObject.hpp"

#include <AL/al.h>
#include <AL/alc.h>


namespace sp
{
namespace audio
{


class ALBufferObject;

class OpenALSound : public Sound
{
    
    public:
        
        OpenALSound();
        ~OpenALSound();
        
        /* Functions */
        
        bool reload(const io::stringc &Filename, u8 BufferCount = DEF_SOUND_BUFFERCOUNT);
        void close();
        
        void play();
        void pause(bool Paused = true);
        void stop();
        
        void emit2D(f32 Volume = 1.0f, bool UseEffectSlot = true);
        void emit3D(const dim::vector3df &Point, f32 Volume = 1.0f, bool UseEffectSlot = true);
        
        void setSeek(f32 Seek);
        f32 getSeek() const;
        
        void setVolume(f32 Volume);
        void setSpeed(f32 Speed);
        void setBalance(f32 Balance);
        
        void setLoop(bool Enable);
        
        bool playing() const;
        bool finish() const;
        
        f32 getLength() const;
        bool valid() const;
        
        void setPosition(const dim::vector3df &Position);
        void setVelocity(const dim::vector3df &Veclotiy);
        
        void setVolumetric(bool isVolumetric);
        void setVolumetricRadius(f32 Radius);
        
    private:
        
        /* Functions */
        
        ALuint nextSourceBuffer();
        
        /* Inline functions */
        
        inline ALuint getSourceID() const
        {
            return (*CurSourceObject_)->getID();
        }
        
        /* Members */
        
        ALBufferObject* BufferObject_;
        
        std::vector<ALSourceObject*> SourceObjects_;
        std::vector<ALuint> SourceObjectIDs_;
        
        std::vector<ALSourceObject*>::iterator CurSourceObject_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
