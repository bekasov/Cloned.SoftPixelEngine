/*
 * Sound effect interface file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/spSoundEffect.hpp"


namespace sp
{
namespace audio
{


SoundEffect::SoundEffect() :
    Type_(SOUNDEFFECT_REVERBERATION)
{
}
SoundEffect::~SoundEffect()
{
}

void SoundEffect::setType(const ESoundEffectTypes Type)
{
    Type_ = Type;
}
void SoundEffect::setupEffectPreset(const ESoundEffectPresets Preset)
{
    // do nothing
}


} // /namespace audio

} // /namespace sp



// ================================================================================
