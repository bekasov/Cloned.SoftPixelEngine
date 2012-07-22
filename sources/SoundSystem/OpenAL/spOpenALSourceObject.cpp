/*
 * OpenAL source object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenAL/spOpenALSourceObject.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "SoundSystem/OpenAL/spOpenALBufferObject.hpp"


namespace sp
{
namespace audio
{


ALSourceObject::ALSourceObject() :
    ALSource_(0)
{
    /* Clear error log */
    alGetError();
    
    /* Generate OpenAL source object */
    alGenSources(1, &ALSource_);
    
    if (alGetError() != AL_NO_ERROR)
    {
        io::Log::error("Could not generate OpenAL source object");
        ALSource_ = 0;
    }
}
ALSourceObject::~ALSourceObject()
{
    if (ALSource_ && alIsSource(ALSource_))
        alDeleteSources(1, &ALSource_);
}

void ALSourceObject::setupBuffer(ALBufferObject* BufferObject)
{
    alSourcei(ALSource_, AL_BUFFER, BufferObject ? BufferObject->getID() : AL_NONE);
}

ALint ALSourceObject::getState() const
{
    ALint State = 0;
    alGetSourcei(ALSource_, AL_SOURCE_STATE, &State);
    return State;
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
