/*
 * Audio stream header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_STREAM_H__
#define __SP_AUDIO_STREAM_H__


#include "Base/spStandard.hpp"
#include "FileFormats/Sound/spSoundLoader.hpp"

#include <vector>


namespace sp
{
namespace audio
{


/**
Audio stream for real-time music playback (e.g. Ogg Vorbis).
\since Version 3.3
\ingroup group_audio
*/
class SP_EXPORT AudioStream
{
    
    public:
        
        virtual ~AudioStream()
        {
        }
        
        /* === Functions === */
        
        virtual bool openFile(const io::stringc &Filename) = 0;
        virtual void closeFile() = 0;
        
        /**
        Fills the specified PCM buffer with new data by streaming the active audio file.
        \param[in,out] BufferPCM Specifies the raw PCM audio buffer. The buffer must nut be empty.
        The audio stream will fill the whole buffer until the end of the stream.
        \return True on success. Otherwise streaming is not possible or the specified buffer is empty.
        */
        virtual bool stream(std::vector<s8> &BufferPCM) = 0;
        
        /* === Inline functions === */
        
        inline EWaveChannelFormats getFormat() const
        {
            return Format_;
        }
        
    protected:
        
        AudioStream() :
            Format_(WAVECHANNEL_MONO8)
        {
        }
        
        /* === Members === */
        
        EWaveChannelFormats Format_;
        
};


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
