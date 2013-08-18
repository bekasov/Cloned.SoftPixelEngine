/*
 * Audio stream "Ogg Vorbis" header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_STREAM_OGG_VORBIS_H__
#define __SP_AUDIO_STREAM_OGG_VORBIS_H__


#include "FileFormats/Sound/spAudioStream.hpp"

#if defined(SP_COMPILE_WITH_AUDIOSTREAM_OGG)


#include <stdio.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>


namespace sp
{
namespace audio
{


//! "Ogg Vorbis" audio stream class.
class SP_EXPORT AudioStreamOGG : public AudioStream
{
    
    public:
        
        AudioStreamOGG();
        ~AudioStreamOGG();
        
        /* === Functions === */
        
        bool openFile(const io::stringc &Filename);
        void closeFile();
        
        bool stream(std::vector<s8> &BufferPCM);
        
    private:
        
        /* === Functions === */
        
        static io::stringc getErrorString(s32 ErrCode);
        
        /* === Members === */
        
        FILE* OggFile_;
        OggVorbis_File OggStream_;
        
        vorbis_info* VorbisInfo_;
        vorbis_comment* VorbisComment_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
