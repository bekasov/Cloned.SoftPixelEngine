/*
 * Sound loader WAV file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Sound/spSoundLoaderWAV.hpp"

#if defined(SP_COMPILE_WITH_SOUNDLOADER_WAV)


#include "Base/spInputOutputLog.hpp"
#include "Framework/Tools/spExtendedToolset.hpp"

#include <stdio.h>


namespace sp
{
namespace audio
{


SoundLoaderWAV::SoundLoaderWAV() :
    SoundLoader()
{
    StrBuffer_[4] = 0;
}
SoundLoaderWAV::~SoundLoaderWAV()
{
}

SAudioBuffer* SoundLoaderWAV::loadSoundData(io::File* File)
{
    if (!File)
        return 0;
    
    File_           = File;
    AudioBuffer_    = MemoryManager::createMemory<SAudioBuffer>("SoundLoaderWAV::AudioBuffer");
    
    if (!readHeader() || !readFormat() || !readBufferPCM())
    {
        MemoryManager::deleteMemory(AudioBuffer_);
        return 0;
    }
    
    return AudioBuffer_;
}


/*
 * ======= Private: =======
 */

bool SoundLoaderWAV::readHeader()
{
    /* Read WAVE magic number */
    File_->readBuffer(StrBuffer_, 4);
    
    if (strcmp(StrBuffer_, "RIFF") != 0)
    {
        io::Log::error("WAVE file has invalid magic number");
        return false;
    }
    
    /* Ignore file size value */
    File_->ignore(4);
    
    /* Read "WAVE" chunk */
    File_->readBuffer(StrBuffer_, 4);
    
    if (strcmp(StrBuffer_, "WAVE") != 0)
    {
        io::Log::error("RIFF type missing in WAVE file");
        return false;
    }
    
    File_->readBuffer(StrBuffer_, 4);
    
    if (strcmp(StrBuffer_, "fmt ") != 0)
    {
        io::Log::error("WAVE file has no \"fmt \" chunk");
        return false;
    }
    
    return true;
}

bool SoundLoaderWAV::readFormat()
{
    /* Read format chunk */
    File_->readBuffer(&Format_, sizeof(SFormatWAV));
    
    /* Check for compression method */
    if (Format_.AudioFormat != WAVEFORMAT_PCM)
    {
        io::Log::error(
            "Unsupported WAVE format: \"" + tool::Debugging::toString(static_cast<EWaveBufferFormats>(Format_.AudioFormat)) + "\""
        );
        return false;
    }
    
    /* Setup audio buffer */
    SWaveFormatFlags* Flags = &(AudioBuffer_->FormatFlags);
    
    Flags->Channels         = Format_.Channels;
    Flags->SamplesPerSec    = Format_.SampleRate;
    Flags->BytePerSec       = Format_.ByteRate;
    Flags->BlockAlign       = Format_.BlockAlign;
    Flags->BitsPerSample    = Format_.BitsPerSample;
    Flags->ChannelFormat    = static_cast<EWaveChannelFormats>(Format_.AudioFormat);
    
    return true;
}

bool SoundLoaderWAV::readBufferPCM()
{
    /* Read next chunk */
    File_->readBuffer(StrBuffer_, 4);
    
    if (strcmp(StrBuffer_, "fact") == 0)
    {
        io::Log::error("Compressed WAVE files are not supported");
        return false;
    }
    
    if (strcmp(StrBuffer_, "data") != 0)
    {
        io::Log::error("\"data\" block missing in WAVE file");
        return false;
    }
    
    /* Read PCM buffer */
    AudioBuffer_->BufferSize    = File_->readValue<u32>();
    AudioBuffer_->BufferPCM     = new s8[AudioBuffer_->BufferSize];
    
    File_->readBuffer(AudioBuffer_->BufferPCM, sizeof(s8), AudioBuffer_->BufferSize);
    
    return true;
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
