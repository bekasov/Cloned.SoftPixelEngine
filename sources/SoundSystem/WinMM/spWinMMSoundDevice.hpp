/*
 * WinMM sound device header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDDEVICE_WINMM_H__
#define __SP_AUDIO_SOUNDDEVICE_WINMM_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_WINMM)


#include "SoundSystem/spSoundDevice.hpp"
#include "SoundSystem/WinMM/spWinMMSound.hpp"


namespace sp
{
namespace audio
{


class SP_EXPORT WinMMSoundDevice : public SoundDevice
{
        
    public:
            
        WinMMSoundDevice();
        ~WinMMSoundDevice();
        
        /* Functions */
        
        io::stringc getInterface() const;
        
        Sound* createSound();
        
    private:
        
        friend class WinMMSound;
        
        /* Functions */
        
        static std::string sendMessageMCI(const io::stringc &Message);
        static void sendMessageMCI(const io::stringc &Message, c8* Buffer, u32 Length);
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
