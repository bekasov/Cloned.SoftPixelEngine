/*
 * XAudio2 sound device header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDDEVICE_XAUDIO2_H__
#define __SP_AUDIO_SOUNDDEVICE_XAUDIO2_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_XAUDIO2)


#include "SoundSystem/spSoundDevice.hpp"
#include "SoundSystem/XAudio2/spXAudio2Sound.hpp"


namespace sp
{
namespace audio
{


class SP_EXPORT XAudio2SoundDevice : public SoundDevice
{
        
    public:
            
        XAudio2SoundDevice();
        ~XAudio2SoundDevice();
        
        /* Functions */
        
        io::stringc getInterface() const;
        
        Sound* createSound();
        
    private:
        
        friend class XAudio2Sound;
        
        /* Functions */
        
        
        
        /* Templates */
        
        template <class T> static inline void releaseObject(T* &Object)
        {
            if (Object)
            {
                Object->Release();
                Object = 0;
            }
        }
        
        /* Members */
        
        IXAudio2* Device_;
        IXAudio2MasteringVoice* MasterVoice_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
