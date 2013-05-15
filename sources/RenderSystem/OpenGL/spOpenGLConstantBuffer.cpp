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
    GLHardwareBuffer(GL_UNIFORM_BUFFER      ),
    ProgramObject_  (Owner->ProgramObject_  ),
    BlockIndex_     (Index                  )
{
    Size_ = getBlockSize();
    
    /*
     * Generate new hardware buffer and
     * allocate enough space for the uniform block
     */
    createBuffer();
    setupBuffer(0, Size_, Usage_);
}
OpenGLConstantBuffer::~OpenGLConstantBuffer()
{
}

bool OpenGLConstantBuffer::updateBuffer(const void* Buffer, u32 Size)
{
    if (!Buffer)
        return false;
    
    /* Get correct buffer size */
    if (!Size || Size > Size_)
        Size = Size_;
    
    /* Update constant buffer data */
    if (HasUsageChanged_)
    {
        setupBuffer(Buffer, Size, Usage_);
        HasUsageChanged_ = false;
    }
    else
        setupBufferSub(Buffer, Size);
    
    return true;
}

bool OpenGLConstantBuffer::valid() const
{
    return hasBuffer();
}

u32 OpenGLConstantBuffer::getBlockSize() const
{
    /* Get uniform block size */
    GLint BlockSize = 0;
    glGetActiveUniformBlockiv(ProgramObject_, BlockIndex_, GL_UNIFORM_BLOCK_DATA_SIZE, &BlockSize);
    
    if (BlockSize > 0)
        return static_cast<u32>(BlockSize);
    
    return 0;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
