/*
 * OpenSL|ES sound file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenSLES/spOpenSLESSound.hpp"

#if defined(SP_COMPILE_WITH_OPENSLES)


#include "SoundSystem/OpenSLES/spOpenSLESSoundDevice.hpp"


namespace sp
{

extern audio::SoundDevice* __spSoundDevice;

namespace audio
{


/*
 * Declarations
 */

#define mcrGetSoundDevice static_cast<OpenSLESSoundDevice*>(__spSoundDevice)


/*
 * OpenSLESSound class
 */

OpenSLESSound::OpenSLESSound() :
    Sound       (   ),
    AudioPlayer_(0  ),
    AudioPlay_  (0  ),
    AudioSeek_  (0  ),
    AudioVolume_(0  )
{
}
OpenSLESSound::~OpenSLESSound()
{
    close();
}

bool OpenSLESSound::reload(const io::stringc &Filename, u8 BufferCount)
{
    if (!checkFile(Filename))
        return false;
    
    close();
    
    Filename_ = Filename;
    
    createAudioPlayer();
    
    io::Log::lowerTab();
    
    return true;
}

void OpenSLESSound::close()
{
    stop();
    
    OpenSLESSoundDevice::releaseObject(AudioPlayer_);

    AudioPlay_      = 0;
    AudioSeek_      = 0;
    AudioVolume_    = 0;
}

void OpenSLESSound::play(bool Looped)
{
    if (AudioPlay_)
    {
        Sound::play(Looped);
        
        if (AudioSeek_)
            (*AudioSeek_)->SetLoop(AudioSeek_, (isLooped_ ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE), 0, SL_TIME_UNKNOWN);
        
        (*AudioPlay_)->SetPlayState(AudioPlay_, SL_PLAYSTATE_PLAYING);
    }
}

void OpenSLESSound::pause(bool Paused)
{
    Sound::pause(Paused);
    if (AudioPlay_)
        (*AudioPlay_)->SetPlayState(AudioPlay_, (isPaused_ ? SL_PLAYSTATE_PAUSED : SL_PLAYSTATE_PLAYING));
}

void OpenSLESSound::stop()
{
    Sound::stop();
    if (AudioPlay_)
        (*AudioPlay_)->SetPlayState(AudioPlay_, SL_PLAYSTATE_STOPPED);
}

void OpenSLESSound::setSeek(f32 Seek)
{
    Seek_ = Seek;
    
    if (AudioSeek_ && AudioPlay_)
    {
        /* Get duration */
        SLmillisecond Duration;
        (*AudioPlay_)->GetDuration(AudioPlay_, &Duration);
        
        #if 1
        io::Log::message("DEB: Duration = " + io::stringc(Duration));
        #endif
        
        /* Set new seek position */
        (*AudioSeek_)->SetPosition(AudioSeek_, static_cast<SLmillisecond>(Seek_ * Duration), SL_SEEKMODE_FAST);
    }
}

f32 OpenSLESSound::getSeek() const
{
    if (!AudioPlay_)
        return Seek_;
    
    /* Get duration */
    SLmillisecond Duration;
    (*AudioPlay_)->GetDuration(AudioPlay_, &Duration);
    
    if (Duration <= 0)
        return Seek_;
    
    /* Get position */
    SLmillisecond Position;
    (*AudioPlay_)->GetPosition(AudioPlay_, &Position);
    
    return static_cast<f32>(Position) / Duration;
}

void OpenSLESSound::setVolume(f32 Volume)
{
    if (Volume_ != Volume)
    {
        Volume_ = Volume;
        
        if (AudioVolume_)
        {
            /* Get maximal volume level */
            SLmillibel VolumeLevel, MaxVolumeLevel;
            (*AudioVolume_)->GetMaxVolumeLevel(AudioVolume_, &MaxVolumeLevel);
            
            #if 1
            (*AudioVolume_)->GetVolumeLevel(AudioVolume_, &VolumeLevel);
            io::Log::message("DEB: MaxVolumeLevel = " + io::stringc(MaxVolumeLevel) + ", SL_MILLIBEL_MIN = " + io::stringc(SL_MILLIBEL_MIN) + ", VolumeLevel = " + io::stringc(VolumeLevel));
            #endif
            
            /* Set new volume level */
            VolumeLevel = SL_MILLIBEL_MIN + static_cast<SLmillibel>(Volume_ * (MaxVolumeLevel - SL_MILLIBEL_MIN));
            (*AudioVolume_)->SetVolumeLevel(AudioVolume_, VolumeLevel);
        }
    }
}

void OpenSLESSound::setSpeed(f32 Speed)
{
    Speed_ = Speed;
    //todo
}

void OpenSLESSound::setBalance(f32 Balance)
{
    Balance_ = Balance;
    //todo
}

f32 OpenSLESSound::getLength() const
{
    SLmillisecond MiliSecs;
    (*AudioPlay_)->GetDuration(AudioPlay_, &MiliSecs);
    return static_cast<f32>(MiliSecs);
}

bool OpenSLESSound::valid() const
{
    return true; //todo
}

bool OpenSLESSound::saveRecord(const io::stringc &Filename)
{
    return true; //todo
}

void OpenSLESSound::beginRecording()
{
    //todo
}

void OpenSLESSound::endRecording()
{
    //todo
}


/*
 * ======= Private: =======
 */

bool OpenSLESSound::createAudioPlayer()
{
    /* Open asset as file descriptor */
    AAsset* Asset = AAssetManager_open(io::FileAsset::AssetManager_, Filename_.c_str(), AASSET_MODE_UNKNOWN);
    
    off_t Start, Length;
    s32 FileDescriptor = AAsset_openFileDescriptor(Asset, &Start, &Length);
    
    AAsset_close(Asset);
    
    /* Configure audio source */
    SLDataLocator_AndroidFD LocatorFD = { SL_DATALOCATOR_ANDROIDFD, FileDescriptor, Start, Length };
    SLDataFormat_MIME FormatMIME = { SL_DATAFORMAT_MIME, 0, SL_CONTAINERTYPE_UNSPECIFIED };
    
    SLDataSource AudioSrc = { &LocatorFD, &FormatMIME };
    
    /* Configure audio sink */
    SLDataLocator_OutputMix LocatorOutput = { SL_DATALOCATOR_OUTPUTMIX, mcrGetSoundDevice->OutputMixer_ };
    SLDataSink AudioSnk = { &LocatorOutput, 0 };
    
    /* Create audio player */
    const SLInterfaceID InterfaceID[2]  = { SL_IID_SEEK, SL_IID_VOLUME };
    const SLboolean Required[2]         = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
    
    if (!OpenSLESSoundDevice::checkForError(
            (*mcrGetSoundDevice->Engine_)->CreateAudioPlayer(
                mcrGetSoundDevice->Engine_, &AudioPlayer_, &AudioSrc, &AudioSnk, 2, InterfaceID, Required
            ),
            "Creating audio player failed"))
    {
        return false;
    }
    
    /* Realize the object */
    if (!OpenSLESSoundDevice::objectRealize(AudioPlayer_, "audio player"))
        return false;
    
    /* Get play, seek and volume interfaces */
    if ( !OpenSLESSoundDevice::objectGetInterface(AudioPlayer_, SL_IID_PLAY, &AudioPlay_, "audio playback") ||
         !OpenSLESSoundDevice::objectGetInterface(AudioPlayer_, SL_IID_SEEK, &AudioSeek_, "audio seek") ||
         !OpenSLESSoundDevice::objectGetInterface(AudioPlayer_, SL_IID_VOLUME, &AudioVolume_, "audio volume") )
    {
        return false;
    }
    
    return true;
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
