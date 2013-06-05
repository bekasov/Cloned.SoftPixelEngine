/*
 * OpenGL shader constant buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_SHADER_CONSTANT_BUFFER_H__
#define __SP_OPENGL_SHADER_CONSTANT_BUFFER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/spConstantBuffer.hpp"
#include "RenderSystem/OpenGL/spOpenGLHardwareBuffer.hpp"
#include "RenderSystem/OpenGL/spOpenGLCoreHeader.hpp"


namespace sp
{
namespace video
{


class OpenGLShader;
class OpenGLShaderClass;

class /*SP_EXPORT */OpenGLConstantBuffer : public ConstantBuffer, public GLHardwareBuffer
{
    
    public:
        
        OpenGLConstantBuffer(OpenGLShaderClass* Owner, const io::stringc &Name, u32 Index);
        ~OpenGLConstantBuffer();
        
        /* === Functions === */
        
        bool updateBuffer(const void* Buffer, u32 Size = 0);
        
        bool valid() const;
        
        u32 getBlockSize() const;

    private:
        
        friend class OpenGLShader;
        
        /* === Members === */
        
        GLuint ProgramObject_;
        u32 BlockIndex_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
