/*
 * WinMM sound file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/WinMM/spWinMMSound.hpp"

#if defined(SP_COMPILE_WITH_WINMM)


#include "SoundSystem/WinMM/spWinMMSoundDevice.hpp"


namespace sp
{
namespace audio
{


const f32 WinMMSound::SOUND_VOLUME_HEIGHT   = 1000.0f;
const f32 WinMMSound::SOUND_SPEED_HEIGHT    = 1000.0f;
const f32 WinMMSound::SOUND_SEEK_HEIGHT     = 1000.0f;

WinMMSound::WinMMSound() :
    Sound   (   ),
    ID_     (0  ),
    CurID_  (0  ),
    RecID_  (0  )
{
    memset(RecBuffer_, 0, sizeof(RecBuffer_));
}
WinMMSound::~WinMMSound()
{
    close();
}

bool WinMMSound::reload(const io::stringc &Filename, u8 BufferCount)
{
    if (!checkFile(Filename))
        return false;
    
    /* Identification */
    static u32 CountID = 1;
    
    ID_ = CountID;
    CountID += BufferCount;
    BufferCount_ = BufferCount;
    
    /* Load the sound */
    for (u32 i = 0, id = ID_; i < BufferCount_; ++i, ++id)
    {
        WinMMSoundDevice::sendMessageMCI(
            io::stringc("OPEN \"") + Filename + io::stringc("\" TYPE MpegVideo ALIAS MMF") + io::stringc(id)
        );
    }
    
    Filename_ = Filename;
    io::Log::lowerTab();
    
    return true;
}

void WinMMSound::close()
{
    for (u32 i = 0; i < BufferCount_; ++i)
        WinMMSoundDevice::sendMessageMCI("CLOSE MMF" + io::stringc(ID_ + i));
}

void WinMMSound::play()
{
    Sound::play();
    
    if (!hasSeekChanged_)
    {
        addIDCount();
        
        const io::stringc strID(getCurID());
        
        WinMMSoundDevice::sendMessageMCI("PLAY MMF" + strID + " FROM 0");
        WinMMSoundDevice::sendMessageMCI("WINDOW MMF" + strID + " STATE HIDE");
    }
    else
        setSeek(Seek_);
}

void WinMMSound::pause(bool Paused)
{
    Sound::pause(Paused);
    
    if (isPaused_)
    {
        for (u32 i = 0; i < BufferCount_; ++i)
            WinMMSoundDevice::sendMessageMCI("PAUSE MMF" + io::stringc(ID_ + i));
    }
    else
    {
        if (!hasSeekChanged_)
        {
            for (u32 i = 0; i < BufferCount_; ++i)
                WinMMSoundDevice::sendMessageMCI("RESUME MMF" + io::stringc(ID_ + i));
        }
        else
            setSeek(Seek_);
    }
}

void WinMMSound::stop()
{
    Sound::stop();
    
    for (u32 i = 0; i < BufferCount_; ++i)
        WinMMSoundDevice::sendMessageMCI("STOP MMF" + io::stringc(ID_ + i));
}

void WinMMSound::setSeek(f32 Seek)
{
    Seek_           = Seek;
    hasSeekChanged_ = true;
    
    if (isPlaying_ && !isPaused_)
    {
        stop();
        
        const io::stringc strID(getCurID());
        WinMMSoundDevice::sendMessageMCI("PLAY MMF" + strID + " FROM " + io::stringc(getLength() * Seek).val<s32>());
        WinMMSoundDevice::sendMessageMCI("WINDOW MMF" + strID + " STATE HIDE");
        
        hasSeekChanged_ = false;
    }
}

f32 WinMMSound::getSeek() const
{
    return io::stringc(
        WinMMSoundDevice::sendMessageMCI("STATUS MMF" + io::stringc(getCurID()) + " POSITION")
    ).val<f32>() / getLength();
}

void WinMMSound::setVolume(f32 Volume)
{
    Volume_ = Volume;
    setBalance(Balance_);
}

void WinMMSound::setSpeed(f32 Speed)
{
    Speed_ = Speed;
    
    const io::stringc SpeedStr(Speed_ * SOUND_SPEED_HEIGHT);
    
    for (u32 i = 0; i < BufferCount_; ++i)
        WinMMSoundDevice::sendMessageMCI("SET MMF" + io::stringc(ID_ + i) + " SPEED " + SpeedStr);
}

void WinMMSound::setBalance(f32 Balance)
{
    Balance_ = Balance;
    
    const s32 VolumeLeft    = static_cast<s32>( math::MinMax(1.0f - Balance, 0.0f, 1.0f) * SOUND_VOLUME_HEIGHT * Volume_ );
    const s32 VolumeRight   = static_cast<s32>( math::MinMax(1.0f + Balance, 0.0f, 1.0f) * SOUND_VOLUME_HEIGHT * Volume_ );
    
    for (u32 i = 0; i < BufferCount_; ++i)
    {
        io::stringc TempStr("SETAUDIO MMF" + io::stringc(ID_ + i));
        
        WinMMSoundDevice::sendMessageMCI(TempStr + " LEFT VOLUME TO " + VolumeLeft);
        WinMMSoundDevice::sendMessageMCI(TempStr + " RIGHT VOLUME TO " + VolumeRight);
    }
}

f32 WinMMSound::getLength() const
{
    return io::stringc(
        WinMMSoundDevice::sendMessageMCI("STATUS MMF" + io::stringc(ID_) + " LENGTH")
    ).val<f32>();
}

bool WinMMSound::valid() const
{
    return ID_ != 0;
}

bool WinMMSound::saveRecord(const io::stringc &Filename)
{
    if (isRecording_)
        endRecording();
    
    WinMMSoundDevice::sendMessageMCI(
        "SAVE RECORD" + io::stringc(RecID_) + " \"" + Filename + "\"",
        RecBuffer_, RECORD_BUFFER_SIZE
    );
    
    return true;
}

void WinMMSound::beginRecording()
{
    if (isRecording_)
        return;
    
    WinMMSoundDevice::sendMessageMCI(
        "OPEN NEW TYPE WAVEAUDIO ALIAS RECORD" + io::stringc(RecID_),
        RecBuffer_, RECORD_BUFFER_SIZE
    );
    WinMMSoundDevice::sendMessageMCI(
        "SET RECORD" + io::stringc(RecID_) + " SAMPLESPERBUFFER 8000 BYTESPERBUFFER 8000",
        0, 0
    );
    WinMMSoundDevice::sendMessageMCI(
        "RECORD RECORD" + io::stringc(RecID_),
        RecBuffer_, RECORD_BUFFER_SIZE
    );
    
    isRecording_ = true;
}

void WinMMSound::endRecording()
{
    if (!isRecording_)
        return;
    
    WinMMSoundDevice::sendMessageMCI("stop RECORD" + io::stringc(RecID_), 0, 0);
    
    isRecording_ = false;
}


/*
 * ======= Private: =======
 */

void WinMMSound::addIDCount()
{
    if (++CurID_ >= BufferCount_)
        CurID_ = 0;
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
