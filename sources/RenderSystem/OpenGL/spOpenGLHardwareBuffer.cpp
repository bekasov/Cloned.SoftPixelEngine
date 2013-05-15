/*
 * OpenGL hardware buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLHardwareBuffer.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"


namespace sp
{
namespace video
{


extern GLenum GLMeshBufferUsage[];

GLHardwareBuffer::GLHardwareBuffer(const GLenum Target) :
    Target_     (Target ),
    HWBuffer_   (0      )
{
}
GLHardwareBuffer::~GLHardwareBuffer()
{
    deleteBuffer();
}


/*
 * ======= Protected: =======
 */

void GLHardwareBuffer::createBuffer()
{
    if (!HWBuffer_)
    {
        /* Generate new hardware buffer */
        glGenBuffersARB(1, &HWBuffer_);
    }
}

void GLHardwareBuffer::deleteBuffer()
{
    if (HWBuffer_)
    {
        /* Delete hardware buffer */
        glDeleteBuffersARB(1, &HWBuffer_);
        HWBuffer_ = 0;
    }
}

void GLHardwareBuffer::setupBuffer(const void* Buffer, u32 Size, const EHWBufferUsage Usage)
{
    if (HWBuffer_)
    {
        glBindBufferARB(Target_, HWBuffer_);
        glBufferDataARB(Target_, Size, Buffer, GLMeshBufferUsage[Usage]);
    }
}

void GLHardwareBuffer::setupBufferSub(const void* Buffer, u32 Size, u32 Offset)
{
    if (HWBuffer_)
    {
        glBindBufferARB(Target_, HWBuffer_);
        glBufferSubDataARB(Target_, Offset, Size, Buffer);
    }
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
