/*
 * Audio stream "Ogg Vorbis" header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Sound/spAudioStreamOGG.hpp"

#if defined(SP_COMPILE_WITH_AUDIOSTREAM_OGG)


#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace audio
{


AudioStreamOGG::AudioStreamOGG() :
    AudioStream()
{
}
AudioStreamOGG::~AudioStreamOGG()
{
    closeFile();
}

bool AudioStreamOGG::openFile(const io::stringc &Filename)
{
    closeFile();
    
    s32 Result = 0;
    
    /* Open file */
    if ( !( OggFile_ = fopen(Filename.c_str(), "rb") ) )
    {
        io::Log::error("Could not open 'Ogg Vorbis' file");
        return false;
    }
    
    /* Open ogg vorbis file stream */
    if ( ( Result = ov_open(OggFile_, &OggStream_, 0, 0) ) < 0 )
    {
        fclose(OggFile_);
        io::Log::error("Could not open 'Ogg Vorbis' stream (" + AudioStreamOGG::getErrorString(Result) + ")");
        return false;
    }
    
    /* Get stream information */
    VorbisInfo_ = ov_info(&OggStream_, -1);
    VorbisComment_ = ov_comment(&OggStream_, -1);
    
    if (VorbisInfo_->channels == 1)
        Format_ = WAVECHANNEL_MONO16;
    else
        Format_ = WAVECHANNEL_STEREO16;
    
    return true;
}

void AudioStreamOGG::closeFile()
{
    if (OggStream_)
        ov_clear(&OggStream_);
    if (OggFile_)
    {
        fclose(OggFile_);
        OggFile_ = 0;
    }
}

bool AudioStreamOGG::stream(std::vector<s8> &BufferPCM)
{
    if (BufferPCM.empty() || !OggFile_)
        return false;
    
    size_t Size = 0;
    s32 Section = 0;
    s32 Result = 0;
    
    while (Size < BufferPCM.size())
    {
        /* Read next stream block */
        Result = ov_read(
            &OggStream_,                // Ogg vorbis stream.
            (&BufferPCM[0]) + Size,     // Pointer to the buffer with offset.
            BufferPCM.size() - Size,    // Typical buffer length is 4096.
            0,                          // 0 for little endian byte packing.
            2,                          // 2 for 16-bit samples.
            1,                          // 1 for signed data.
            &Section                    // Pointer to current logical bitstream.
        );
        
        /* Evaluate stream results */
        if (Result > 0)
            Size += Result;
        else if (Result < 0)
        {
            io::Log::error("Streaming 'Ogg Vorbis' file failed (" + AudioStreamOGG::getErrorString(Result) + ")");
            return false;
        }
        else
            break;
    }
    
    //alBufferData(buffer, format, data, size, vorbisInfo->rate);
    
    return Size > 0;
}


/*
 * ======= Private: =======
 */

io::stringc AudioStreamOGG::getErrorString(s32 ErrCode)
{
    switch (ErrCode)
    {
        case OV_EREAD:
            return "Read from media";
        case OV_ENOTVORBIS:
            return "Not Vorbis data";
        case OV_EVERSION:
            return "Vorbis version mismatch";
        case OV_EBADHEADER:
            return "Invalid Vorbis header";
        case OV_EFAULT:
            return "Internal logic fault; bug or heap/stack corruption";
        default:
            return "Unknown Ogg error";
    }
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
