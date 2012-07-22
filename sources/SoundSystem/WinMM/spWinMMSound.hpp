/*
 * WinMM sound header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUND_WINMM_H__
#define __SP_AUDIO_SOUND_WINMM_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_WINMM)


#include "SoundSystem/spSound.hpp"


namespace sp
{
namespace audio
{


class SP_EXPORT WinMMSound : public Sound
{
    
    public:
        
        WinMMSound();
        ~WinMMSound();
        
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
        
        bool saveRecord(const io::stringc &Filename);
        
        void beginRecording();
        void endRecording();
        
    private:
        
        /* Macros */
        
        static const f32 SOUND_VOLUME_HEIGHT;
        static const f32 SOUND_SPEED_HEIGHT;
        static const f32 SOUND_SEEK_HEIGHT;
        
        static const s32 RECORD_BUFFER_SIZE     = 128;
        
        /* Inline functions */
        
        inline u32 getCurID() const
        {
            return ID_ + CurID_;
        }
        
        /* Functions */
        
        void init();
        
        void addIDCount();
        
        /* Members */
        
        u32 ID_, CurID_;
        
        u32 RecID_;
        c8 RecBuffer_[RECORD_BUFFER_SIZE];
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
