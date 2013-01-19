/*
 * Dummy sound header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUND_DUMMY_H__
#define __SP_AUDIO_SOUND_DUMMY_H__


#include "Base/spStandard.hpp"
#include "SoundSystem/spSound.hpp"


namespace sp
{
namespace audio
{


class SP_EXPORT DummySound : public Sound
{
    
    public:
        
        DummySound();
        ~DummySound();
        
        /* Functions */
        
        bool reload(const io::stringc &Filename, u8 BufferCount = DEF_SOUND_BUFFERCOUNT);
        void close();
        
        void setSeek(f32 Seek);
        f32 getSeek() const;
        
        void setVolume(f32 Volume);
        void setSpeed(f32 Speed);
        void setBalance(f32 Balance);
        
        f32 getLength() const;
        bool valid() const;
        
};


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
