/*
 * WinMM sound device file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/WinMM/spWinMMSoundDevice.hpp"

#if defined(SP_COMPILE_WITH_WINMM)

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#   include <mmsystem.h>
#endif


namespace sp
{
namespace audio
{


WinMMSoundDevice::WinMMSoundDevice() : SoundDevice(SOUNDDEVICE_WINMM)
{
}
WinMMSoundDevice::~WinMMSoundDevice()
{
}

io::stringc WinMMSoundDevice::getInterface() const
{
    return "WinMM";
}

Sound* WinMMSoundDevice::createSound()
{
    WinMMSound* NewSound = new WinMMSound();
    SoundList_.push_back(NewSound);
    return NewSound;
}


/*
 * ======= Private: =======
 */

static const s32 SOUND_MCI_STRLEN = 256;

std::string WinMMSoundDevice::sendMessageMCI(const io::stringc &Message)
{
    char Result[SOUND_MCI_STRLEN];
    mciSendString(Message.c_str(), Result, SOUND_MCI_STRLEN, 0);
    return std::string(Result);
}

void WinMMSoundDevice::sendMessageMCI(const io::stringc &Message, c8* Buffer, u32 Length)
{
    mciSendString(Message.c_str(), Buffer, Length, 0);
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
