/*
 * Sound loader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDLOADER_H__
#define __SP_AUDIO_SOUNDLOADER_H__


#include "Base/spStandard.hpp"
#include "Base/spBaseFileFormatHandler.hpp"

#include <boost/shared_ptr.hpp>


namespace sp
{
namespace audio
{


//! Audio wave buffer formats (Values and names found on wikipedia.org).
enum EWaveBufferFormats
{
    WAVEFORMAT_PCM                      = 0x0001, //!< Only uncompressed and supported wave format.
    WAVEFORMAT_MS_ADPCM                 = 0x0002,
    WAVEFORMAT_IEEE_FLOAT               = 0x0003,
    WAVEFORMAT_IBM_CVSD                 = 0x0005,
    WAVEFORMAT_ALAW                     = 0x0006,
    WAVEFORMAT_MULAW                    = 0x0007,
    WAVEFORMAT_OKI_ADPCM                = 0x0010,
    WAVEFORMAT_DVI_IMA_ADPCM            = 0x0011,
    WAVEFORMAT_MEDIASPACE_ADPCM         = 0x0012,
    WAVEFORMAT_SIERRA_ADPCM             = 0x0013,
    WAVEFORMAT_G723_ADPCM               = 0x0014,
    WAVEFORMAT_DIGISTD                  = 0x0015,
    WAVEFORMAT_DIGIFIX                  = 0x0016,
    WAVEFORMAT_DIALOGIC_OKI_ADPCM       = 0x0017,
    WAVEFORMAT_YAMAHA_ADPCM             = 0x0020,
    WAVEFORMAT_SONARC                   = 0x0021,
    WAVEFORMAT_DSPGROUP_TRUESPEECH      = 0x0022,
    WAVEFORMAT_ECHOSC1                  = 0x0023,
    WAVEFORMAT_AUDIOFILE_AF36           = 0x0024,
    WAVEFORMAT_APTX                     = 0x0025,
    WAVEFORMAT_AUDIOFILE_AF10           = 0x0026,
    WAVEFORMAT_DOLBY_AC2                = 0x0030,
    WAVEFORMAT_GSM610                   = 0x0031,
    WAVEFORMAT_ANTEX_ADPCME             = 0x0033,
    WAVEFORMAT_CONTROL_RES_VQLPC1       = 0x0034,
    WAVEFORMAT_CONTROL_RES_VQLPC2       = 0x0035,
    WAVEFORMAT_DIGIADPCM                = 0x0036,
    WAVEFORMAT_CONTROL_RES_CR10         = 0x0037,
    WAVEFORMAT_NMS_VBXADPCM             = 0x0038,
    WAVEFORMAT_CS_IMAADPCM              = 0x0039,
    WAVEFORMAT_G721_ADPCM               = 0x0040,
    WAVEFORMAT_MPEG_1_Layer_I_II        = 0x0050,
    WAVEFORMAT_MPEG_1_Layer_III         = 0x0055,
    WAVEFORMAT_XBOX_ADPCM               = 0x0069,
    WAVEFORMAT_CREATIVE_ADPCM           = 0x0200,
    WAVEFORMAT_CREATIVE_FASTSPEECH8     = 0x0202,
    WAVEFORMAT_CREATIVE_FASTSPEECH10    = 0x0203,
    WAVEFORMAT_FM_TOWNS_SND             = 0x0300,
    WAVEFORMAT_OLIGSM                   = 0x1000,
    WAVEFORMAT_OLIADPCM                 = 0x1001,
    WAVEFORMAT_OLICELP                  = 0x1002,
    WAVEFORMAT_OLISBC                   = 0x1003,
    WAVEFORMAT_OLIOPR                   = 0x1004,
};

//! Audio wave channels formats.
enum EWaveChannelFormats
{
    WAVECHANNEL_MONO8 = 0,  //!< 8 bit mono channel.
    WAVECHANNEL_MONO16,     //!< 16 bit mono channel.
    WAVECHANNEL_STEREO8,    //!< 8 bit stereo channels.
    WAVECHANNEL_STEREO16,   //!< 16 bit stereo channels.
};


//! Wave sound format structure.
struct SWaveFormatFlags
{
    SWaveFormatFlags() :
        Channels        (1                  ),
        SamplesPerSec   (0                  ),
        BytePerSec      (8                  ),
        BlockAlign      (0                  ),
        BitsPerSample   (8                  ),
        ChannelFormat   (WAVECHANNEL_MONO8  )
    {
    }
    ~SWaveFormatFlags()
    {
    }
    
    /* Members */
    s16 Channels;
    s32 SamplesPerSec;
    s32 BytePerSec;
    s16 BlockAlign;
    s16 BitsPerSample;
    EWaveChannelFormats ChannelFormat;
};

//! Loaded audio buffer structure.
struct SAudioBuffer
{
    SAudioBuffer() :
        BufferPCM   (0),
        BufferSize  (0)
    {
    }
    ~SAudioBuffer()
    {
        MemoryManager::deleteBuffer(BufferPCM);
    }
    
    /* Members */
    s8* BufferPCM;                  //!< Buffer data in PCM format.
    u32 BufferSize;                 //!< Buffer size in bytes.
    SWaveFormatFlags FormatFlags;   //!< Format flags (count of channels, samples per seconds etc.).
};

//! Shared pointer type to the SAudioBuffer structure. Used for temporary audio buffer data.
typedef boost::shared_ptr<SAudioBuffer> SAudioBufferPtr;


class SP_EXPORT SoundLoader : public io::BaseFileFormatHandler
{
    
    public:
        
        virtual ~SoundLoader()
        {
        }
        
        /* Functions */
        
        virtual SAudioBufferPtr loadSoundData(io::File* File) = 0;
        
    protected:
        
        SoundLoader() : io::BaseFileFormatHandler()
        {
        }
        
        /* Members */
        
        SAudioBufferPtr AudioBuffer_;
        
};


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
