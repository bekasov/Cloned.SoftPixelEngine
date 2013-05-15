/*
 * OpenGL texture buffer object (TBO) header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_TBO_H__
#define __SP_OPENGL_TBO_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/spTextureBase.hpp"
#include "RenderSystem/OpenGL/spOpenGLCoreHeader.hpp"
#include "RenderSystem/OpenGL/spOpenGLHardwareBuffer.hpp"


namespace sp
{
namespace video
{


//! OpenGL TextureBufferObject (TBO). For internal use only.
class SP_EXPORT GLTextureBufferObject : public GLHardwareBuffer
{
    
    public:
        
        GLTextureBufferObject();
        ~GLTextureBufferObject();
        
        /* === Functions === */
        
        bool attachBuffer(
            const void* Buffer, u32 Size, const EPixelFormats Format, const EHWTextureFormats DataType
        );
        void detachBuffer();
        
    private:
        
        /* === Functions === */
        
        GLenum getInternalFormat(const EPixelFormats Format, const EHWTextureFormats DataType) const;

        /* === Members === */
        
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
