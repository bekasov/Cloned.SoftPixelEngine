/*
 * OpenGL shader resource file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLShaderResource.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


OpenGLShaderResource::OpenGLShaderResource() :
    ShaderResource      (                           ),
    GLHardwareBuffer    (GL_SHADER_STORAGE_BUFFER   ),
    BufferSize_         (0                          ),
    AtomicCounterBuffer_(GL_ATOMIC_COUNTER_BUFFER   )
{
}
OpenGLShaderResource::~OpenGLShaderResource()
{
}

bool OpenGLShaderResource::setupBufferRaw(
    const EShaderResourceTypes Type, u8 AccessFlags, u32 ElementCount, u32 Stride,
    const ERendererDataTypes DataType, u32 DataSize, const void* Buffer)
{
    if (!validateParameters(AccessFlags, ElementCount, Stride))
        return false;

    /* Store new settings */
    BufferSize_ = ElementCount * Stride;
    
    /* Create and initialize GL hardware buffer */
    createBuffer();
    GLHardwareBuffer::setupBuffer(Buffer, BufferSize_, HWBUFFER_STATIC);
    
    /* Create atomic counter */
    if (Type_ == SHADERRESOURCE_COUNTER_STRUCT_BUFFER)
        AtomicCounterBuffer_.createBuffer();
    else
        AtomicCounterBuffer_.deleteBuffer();
    
    return true;
}

bool OpenGLShaderResource::writeBuffer(const void* Buffer, u32 Size)
{
    if (Buffer)
    {
        if (Size > 0)
            GLHardwareBuffer::setupBufferSub(Buffer, Size);
        else
            GLHardwareBuffer::setupBufferSub(Buffer, BufferSize_);
        return true;
    }
    return false;
}

bool OpenGLShaderResource::readBuffer(void* Buffer, u32 Size)
{
    //todo ...
    return false;
}

bool OpenGLShaderResource::copyBuffer(const ShaderResource* SourceBuffer)
{
    //todo ...
    return false;
}

u32 OpenGLShaderResource::getSize() const
{
    return BufferSize_;
}


/*
 * ======= Private: =======
 */



} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
