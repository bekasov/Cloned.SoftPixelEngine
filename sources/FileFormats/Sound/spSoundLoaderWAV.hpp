/*
 * Sound loader WAV header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDLOADER_WAV_H__
#define __SP_AUDIO_SOUNDLOADER_WAV_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_SOUNDLOADER_WAV)


#include "FileFormats/Sound/spSoundLoader.hpp"


namespace sp
{
namespace audio
{


class SP_EXPORT SoundLoaderWAV : public SoundLoader
{
    
    public:
        
        SoundLoaderWAV();
        ~SoundLoaderWAV();
        
        /* Functions */
        
        SAudioBuffer* loadSoundData(io::File* File);
        
    private:
        
        /* Structures */
        
        #if defined(_MSC_VER)
        #   pragma pack(push, packing)
        #   pragma pack(1)
        #   define SP_PACK_STRUCT
        #elif defined(__GNUC__)
        #   define SP_PACK_STRUCT __attribute__((packed))
        #else
        #   define SP_PACK_STRUCT
        #endif
        
        struct SFormatWAV
        {
            s32 FormatLength;
            s16 AudioFormat;
            s16 Channels;
            s32 SampleRate;
            s32 ByteRate;
            s16 BlockAlign;
            s16 BitsPerSample;
        }
        SP_PACK_STRUCT;
        
        #ifdef _MSC_VER
        #   pragma pack(pop, packing)
        #endif
        
        #undef SP_PACK_STRUCT
        
        /* Functions */
        
        bool readHeader();
        bool readFormat();
        bool readBufferPCM();
        
        /* Members */
        
        SFormatWAV Format_;
        c8 StrBuffer_[5];
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
