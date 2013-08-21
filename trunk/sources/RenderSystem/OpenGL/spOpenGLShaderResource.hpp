/*
 * OpenGL shader resource header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_SHADER_RESOURCE_H__
#define __SP_OPENGL_SHADER_RESOURCE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/spShaderResource.hpp"
#include "RenderSystem/OpenGL/spOpenGLHardwareBuffer.hpp"


namespace sp
{
namespace video
{


//! OpenGL shader resource (Equivalent to the Direct3D 11 shader buffers).
class OpenGLShaderResource : public ShaderResource, public GLHardwareBuffer
{
    
    public:
        
        OpenGLShaderResource();
        ~OpenGLShaderResource();
        
        /* === Functions === */
        
        bool setupBufferRaw(
            const EShaderResourceTypes Type, u32 ElementCount, u32 Stride,
            const ERendererDataTypes DataType, u32 DataSize, const void* Buffer = 0
        );
        
        bool writeBuffer(const void* Buffer, u32 Size = 0);
        bool readBuffer(void* Buffer, u32 Size = 0);

        bool copyBuffer(const ShaderResource* SourceBuffer);

        u32 getSize() const;

    private:

        /* === Functions === */
        
        
        
        /* === Members === */
        
        u32 BufferSize_;
        
        GLHardwareBuffer AtomicCounterBuffer_;

};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
