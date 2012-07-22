/*
 * OpenGL|ES 1 texture header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_OPENGLES1TEXTURE_H__
#define __SP_RENDERSYSTEM_OPENGLES1TEXTURE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES1)


#include "RenderSystem/OpenGL/spOpenGLTextureBase.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT OpenGLES1Texture : public GLTextureBase
{
    
    public:
        
        OpenGLES1Texture();
        OpenGLES1Texture(const STextureCreationFlags &CreationFlags);
        ~OpenGLES1Texture();
        
        /* Functions */
        
        void updateImageBuffer();
        
    private:
        
        friend class OpenGLES1RenderSystem;
        
        /* Functions */
        
        void updateFormat();
        
        void setupTextureFormats(
            const EPixelFormats Format, const EHWTextureFormats HWFormat, GLenum &GLFormat, GLenum &GLInternalFormat
        );
        
        void updateTextureImageNormal(
            dim::vector3di Size, s32 FormatSize, GLenum GLInternalFormat, GLenum GLFormat,
            const u8* ImageBuffer, s32 Level
        );
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
