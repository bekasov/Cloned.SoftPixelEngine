/*
 * Dummy sound file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/spDummySoundDevice.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


namespace sp
{
namespace audio
{


DummySoundDevice::DummySoundDevice() : SoundDevice(SOUNDDEVICE_DUMMY)
{
}
DummySoundDevice::~DummySoundDevice()
{
}

io::stringc DummySoundDevice::getInterface() const
{
    return "Dummy";
}

DummySound* DummySoundDevice::createSound()
{
    DummySound* NewSound = new DummySound();
    SoundList_.push_back(NewSound);
    return NewSound;
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
