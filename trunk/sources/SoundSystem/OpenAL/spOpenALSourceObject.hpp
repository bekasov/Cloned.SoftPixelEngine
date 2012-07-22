/*
 * OpenAL source object header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_OPENAL_SOURCE_H__
#define __SP_AUDIO_OPENAL_SOURCE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include <AL/al.h>
#include <AL/alc.h>


namespace sp
{
namespace audio
{


class ALBufferObject;

//! Internal OpenAL source object abstraction layer.
class ALSourceObject
{
    
    public:
        
        ALSourceObject();
        ~ALSourceObject();
        
        /* Functions */
        
        void setupBuffer(ALBufferObject* BufferObject);
        
        ALint getState() const;
        
        /* Inline functions */
        
        //! Returns the OpenAL source object ID.
        inline ALuint getID() const
        {
            return ALSource_;
        }
        
    private:
        
        /* Members */
        
        ALuint ALSource_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
