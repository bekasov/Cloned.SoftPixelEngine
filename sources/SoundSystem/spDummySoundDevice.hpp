/*
 * Dummy sound header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDDEVICE_DUMMY_H__
#define __SP_AUDIO_SOUNDDEVICE_DUMMY_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "SoundSystem/spSoundDevice.hpp"
#include "SoundSystem/spDummySound.hpp"


namespace sp
{
namespace audio
{


class SP_EXPORT DummySoundDevice : public SoundDevice
{
        
    public:
            
        DummySoundDevice();
        ~DummySoundDevice();
        
        /* Functions */
        
        io::stringc getInterface() const;
        
        DummySound* createSound();
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
