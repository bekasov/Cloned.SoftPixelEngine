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


const u32 SoundLoaderWAV::CHUNK_ID_FMT  = *((u32*)"fmt ");
const u32 SoundLoaderWAV::CHUNK_ID_DATA = *((u32*)"data");

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
    
    if (!readHeader() || !readChunks())
    {
        MemoryManager::deleteMemory(AudioBuffer_);
        return 0;
    }
    
    return AudioBuffer_;
}


/*
 * ======= Private: =======
 */

bool SoundLoaderWAV::readChunk(SChunkWAV &Chunk)
{
    /* Read chunk block */
    return File_->readBuffer(&Chunk, sizeof(Chunk)) == sizeof(Chunk);
}

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
    
    return true;
}

bool SoundLoaderWAV::readChunks()
{
    SChunkWAV Chunk;
    
    bool HasChunkFmt = false, HasChunkData = false;
    
    /* Read all chunk blocks */
    while ( readChunk(Chunk) && !File_->isEOF() && ( !HasChunkFmt || !HasChunkData ) )
    {
        if (Chunk.ChunkID == SoundLoaderWAV::CHUNK_ID_FMT)
        {
            if (readChunkFmt(Chunk))
                HasChunkFmt = true;
            else
                return false;
        }
        else if (Chunk.ChunkID == SoundLoaderWAV::CHUNK_ID_DATA)
        {
            if (readChunkData(Chunk))
                HasChunkData = true;
            else
                return false;
        }
        else
            File_->ignore(Chunk.ChunkSize);
    }
    
    /* Check for missing chunks */
    if (!HasChunkFmt)
    {
        io::Log::error("WAVE file has no \"fmt \" chunk");
        return false;
    }
    
    if (!HasChunkData)
    {
        io::Log::error("WAVE file has no \"data\" chunk");
        return false;
    }
    
    return true;
}

bool SoundLoaderWAV::readChunkFmt(const SChunkWAV &Chunk)
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

bool SoundLoaderWAV::readChunkData(const SChunkWAV &Chunk)
{
    /* Read PCM buffer */
    AudioBuffer_->BufferSize    = Chunk.ChunkSize;
    AudioBuffer_->BufferPCM     = new s8[AudioBuffer_->BufferSize];
    
    File_->readBuffer(AudioBuffer_->BufferPCM, sizeof(s8), AudioBuffer_->BufferSize);
    
    return true;
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
