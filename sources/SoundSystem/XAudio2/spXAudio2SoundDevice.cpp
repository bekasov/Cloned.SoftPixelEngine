/*
 * XAudio2 sound device file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/XAudio2/spXAudio2SoundDevice.hpp"

#if defined(SP_COMPILE_WITH_XAUDIO2)

#include "Platform/spSoftPixelDeviceOS.hpp"

#include <windows.h>


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;

namespace audio
{


XAudio2SoundDevice::XAudio2SoundDevice() :
    SoundDevice(SOUNDDEVICE_XAUDIO2), Device_(0), MasterVoice_(0)
{
    /* Initialize XAudio2 */
    #ifndef SP_PLATFORM_XBOX
    CoInitializeEx(0, COINIT_MULTITHREADED);
    #endif
    
    /* Create XAudio2 device */
    UINT32 Flags = 0;
    
    #ifdef _DEBUG
    Flags |= XAUDIO2_DEBUG_ENGINE;
    #endif
    
    if (XAudio2Create(&Device_, Flags) != S_OK)
    {
        io::Log::error("Could not create XAudio2 device");
        return;
    }
    
    /* Create mastering voice */
    if (Device_->CreateMasteringVoice(&MasterVoice_) != S_OK)
    {
        io::Log::error("Could not create mastering voice");
        return;
    }
}
XAudio2SoundDevice::~XAudio2SoundDevice()
{
    if (MasterVoice_)
        MasterVoice_->DestroyVoice();
    
    releaseObject(Device_);
    
    /* DeInitialize XAduio2 */
    #ifndef SP_PLATFORM_XBOX
    CoUninitialize();
    #endif
}

io::stringc XAudio2SoundDevice::getInterface() const
{
    return "XAudio2";
}

Sound* XAudio2SoundDevice::createSound()
{
    return 0;
}


/*
 * ======= Private: =======
 */


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
