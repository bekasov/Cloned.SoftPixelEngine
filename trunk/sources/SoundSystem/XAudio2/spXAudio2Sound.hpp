/*
 * XAudio2 sound header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUND_XAUDIO2_H__
#define __SP_AUDIO_SOUND_XAUDIO2_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_XAUDIO2)


#include "SoundSystem/spSound.hpp"

#include <xaudio2.h>


namespace sp
{
namespace audio
{


class SP_EXPORT XAudio2Sound : public Sound
{
    
    public:
        
        XAudio2Sound();
        ~XAudio2Sound();
        
        /* Functions */
        
        bool reload(const io::stringc &Filename, u8 BufferCount = DEF_SOUND_BUFFERCOUNT);
        void close();
        
        void play();
        void pause(bool Paused = true);
        void stop();
        
        void setSeek(f32 Seek);
        f32 getSeek() const;
        
        void setVolume(f32 Volume);
        void setSpeed(f32 Speed);
        void setBalance(f32 Balance);
        
        f32 getLength() const;
        bool valid() const;
        
    private:
        
        /* Functions */
        
        
        /* Inline functions */
        
        inline s16 getBlockAlign(s16 SoundChannels, s16 BitsPerSecond) const
        {
            return SoundChannels * BitsPerSecond / 8;
        }
        
        /* Members */
        
        IXAudio2SourceVoice* SoruceVoice_;
        
        BYTE* WaveBuffer_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
