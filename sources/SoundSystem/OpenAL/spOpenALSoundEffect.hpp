/*
 * OpenAL sound effect header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDEFFECT_OPENAL_H__
#define __SP_AUDIO_SOUNDEFFECT_OPENAL_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "SoundSystem/spSoundEffect.hpp"

#include <AL/al.h>
#include <AL/alc.h>


namespace sp
{
namespace audio
{


class OpenALSoundEffect : public SoundEffect
{
    
    public:
        
        OpenALSoundEffect();
        ~OpenALSoundEffect();
        
        /* Functions */
        
        void setType(const ESoundEffectTypes Type);
        
        void setupEffectPreset(const ESoundEffectPresets Preset);
        
    private:
        
        friend class OpenALSound;
        
        /* Members */
        
        ALuint ALEffectSlot_;
        ALuint ALEffect_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
