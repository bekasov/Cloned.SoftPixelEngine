/*
 * OpenSL|ES sound header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUND_OPENSLES_H__
#define __SP_AUDIO_SOUND_OPENSLES_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENSLES)


#include "SoundSystem/spSound.hpp"

#include <SLES/OpenSLES.h>

#if defined(SP_PLATFORM_ANDROID)
#   include <SLES/OpenSLES_Android.h>
#   include <SLES/OpenSLES_AndroidConfiguration.h>
#endif


namespace sp
{
namespace audio
{


class SP_EXPORT OpenSLESSound : public Sound
{
    
    public:
        
        OpenSLESSound();
        ~OpenSLESSound();
        
        /* Functions */
        
        bool reload(const io::stringc &Filename, u8 BufferCount = DEF_SOUND_BUFFERCOUNT);
        void close();
        
        void play(bool Looped = false);
        void pause(bool Paused = true);
        void stop();
        
        void setSeek(f32 Seek);
        f32 getSeek() const;
        
        void setVolume(f32 Volume);
        void setSpeed(f32 Speed);
        void setBalance(f32 Balance);
        
        f32 getLength() const;
        bool valid() const;
        
        bool saveRecord(const io::stringc &Filename);
        
        void beginRecording();
        void endRecording();
        
    private:
        
        /* Functions */
        
        bool createAudioPlayer();
        
        /* Members */
        
        SLObjectItf AudioPlayer_;
        SLPlayItf AudioPlay_;
        SLSeekItf AudioSeek_;
        SLVolumeItf AudioVolume_;
        
        //bool isStream_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
