/*
 * OpenGL shader constant buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLConstantBuffer.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLShaderClass.hpp"
#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"


namespace sp
{
namespace video
{


extern GLenum GLMeshBufferUsage[];

OpenGLConstantBuffer::OpenGLConstantBuffer(
    OpenGLShaderClass* Owner, const io::stringc &Name, u32 Index) :
    ConstantBuffer  (Owner, Name, Index     ),
    HWBuffer_       (0                      ),
    ProgramObject_  (Owner->ProgramObject_  )
{
    /* Get uniform block size */
    GLint BlockSize = 0;
    glGetActiveUniformBlockiv(ProgramObject_, Index, GL_UNIFORM_BLOCK_DATA_SIZE, &BlockSize);
    
    if (BlockSize > 0)
        Size_ = static_cast<u32>(BlockSize);
    
    /* Generate new hardware buffer */
    glGenBuffersARB(1, &HWBuffer_);
    
    /* Allocate enough space for the uniform block */
    glBindBufferARB(GL_UNIFORM_BUFFER, HWBuffer_);
    glBufferDataARB(GL_UNIFORM_BUFFER, Size_, 0, GLMeshBufferUsage[Usage_]);
}
OpenGLConstantBuffer::~OpenGLConstantBuffer()
{
    /* Delete hardware buffer */
    glDeleteBuffersARB(1, &HWBuffer_);
}

bool OpenGLConstantBuffer::updateBuffer(const void* Buffer, u32 Size)
{
    if (!Buffer)
        return false;
    
    /* Get correct buffer size */
    if (!Size || Size > Size_)
        Size = Size_;
    
    /* Update constant buffer data */
    glBindBufferARB(GL_UNIFORM_BUFFER, HWBuffer_);
    
    if (HasUsageChanged_)
    {
        glBufferDataARB(GL_UNIFORM_BUFFER, Size, Buffer, GLMeshBufferUsage[Usage_]);
        HasUsageChanged_ = false;
    }
    else
        glBufferSubDataARB(GL_UNIFORM_BUFFER, 0, Size, Buffer);

    return true;
}

bool OpenGLConstantBuffer::valid() const
{
    return HWBuffer_ != 0;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
