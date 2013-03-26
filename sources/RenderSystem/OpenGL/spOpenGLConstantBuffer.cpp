/*
 * OpenGL shader constant buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLConstantBuffer.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLShader.hpp"
#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"


namespace sp
{
namespace video
{


extern GLenum GLMeshBufferUsage[];

OpenGLConstantBuffer::OpenGLConstantBuffer(OpenGLShader* Owner, const io::stringc &Name) :
    ConstantBuffer  (Owner, Name),
    HWBuffer_       (0          )
{
    glGenBuffersARB(1, &HWBuffer_);
}
OpenGLConstantBuffer::~OpenGLConstantBuffer()
{
    glDeleteBuffersARB(1, &HWBuffer_);
}

bool OpenGLConstantBuffer::updateBuffer(const void* Buffer, u32 Size)
{
    if (!Buffer)
        return false;
    
    #ifdef GL_ARB_uniform_buffer_object
    
    /* Get correct buffer size */
    if (!Size)
    {
        //...
    }
    
    /* Update constant buffer data */
    glBindBufferARB(GL_UNIFORM_BUFFER, HWBuffer_);
    glBufferDataARB(GL_UNIFORM_BUFFER, Size, Buffer, GLMeshBufferUsage[Usage_]);
    
    return true;
    
    #else
    
    io::Log::debug("OpenGLConstantBuffer::updateBuffer", "Engine was not compiled with OpenGL extension \"GL_ARB_uniform_buffer_object\"");
    return false;
    
    #endif
}

bool OpenGLConstantBuffer::valid() const
{
    return HWBuffer_ != 0;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
