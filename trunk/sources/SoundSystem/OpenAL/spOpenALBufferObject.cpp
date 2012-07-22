/*
 * OpenAL buffer object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenAL/spOpenALBufferObject.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace audio
{


ALBufferObject::ALBufferObject() :
    ALBuffer_           (0),
    BufferSize_         (0),
    ReferenceCounter_   (0)
{
    /* Clear error log */
    alGetError();
    
    /* Generate OpenAL buffer object */
    alGenBuffers(1, &ALBuffer_);
    
    if (alGetError() != AL_NO_ERROR)
    {
        io::Log::error("Could not generate OpenAL buffer object");
        ALBuffer_ = 0;
    }
}
ALBufferObject::~ALBufferObject()
{
    if (ALBuffer_ && alIsBuffer(ALBuffer_))
        alDeleteBuffers(1, &ALBuffer_);
}

bool ALBufferObject::updateBuffer(
    const io::stringc &Filename, ALenum Format, const void* Buffer, ALsizei Size, ALsizei Frequency)
{
    Filename_ = Filename;
    
    if (ALBuffer_)
    {
        alBufferData(ALBuffer_, Format, Buffer, Size, Frequency);
        return alGetError() == AL_NO_ERROR;
    }
    
    return false;
}

void ALBufferObject::grab()
{
    ++ReferenceCounter_;
}
bool ALBufferObject::drop()
{
    if (ReferenceCounter_ > 0)
    {
        --ReferenceCounter_;
        return ReferenceCounter_ == 0;
    }
    else
        io::Log::error("Cannot drop reference counted OpenAL buffer object");
    
    return false;
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
