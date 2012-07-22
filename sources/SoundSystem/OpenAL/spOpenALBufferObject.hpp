/*
 * OpenAL buffer object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_OPENAL_BUFFER_H__
#define __SP_AUDIO_OPENAL_BUFFER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "FileFormats/Sound/spSoundLoader.hpp"

#include <AL/al.h>
#include <AL/alc.h>


namespace sp
{
namespace audio
{


//! Internal OpenAL buffer object abstraction layer.
class ALBufferObject
{
    
    public:
        
        ALBufferObject();
        ~ALBufferObject();
        
        /* === Functions === */
        
        bool updateBuffer(
            const io::stringc &Filename, ALenum Format, const void* Buffer, ALsizei Size, ALsizei Frequency
        );
        
        //! Increments the reference counter.
        void grab();
        //! Decrements the reference counter and returns true if the reference counter is 0.
        bool drop();
        
        /* === Inline functions === */
        
        //! Retunrs the OpenAL buffer object ID.
        inline ALuint getID() const
        {
            return ALBuffer_;
        }
        
        //! Returns the format flags.
        inline SWaveFormatFlags getFormatFlags() const
        {
            return FormatFlags_;
        }
        inline io::stringc getFilename() const
        {
            return Filename_;
        }
        
    private:
        
        /* === Members === */
        
        ALuint ALBuffer_;               //!< OpenAL buffer object ID.
        
        u32 BufferSize_;
        SWaveFormatFlags FormatFlags_;
        
        u32 ReferenceCounter_;          //!< Counter how often this object is referenced by another object.
        io::stringc Filename_;          //!< Sound filename.
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
