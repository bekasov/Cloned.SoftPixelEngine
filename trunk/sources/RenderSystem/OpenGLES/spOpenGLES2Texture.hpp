/*
 * OpenGL|ES 2 texture header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_OPENGLES2TEXTURE_H__
#define __SP_RENDERSYSTEM_OPENGLES2TEXTURE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES2)


#include "RenderSystem/OpenGL/spOpenGLTextureBase.hpp"
#include "RenderSystem/OpenGL/spOpenGLFramebufferObject.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT OpenGLES2Texture : public GLTextureBase, public GLFramebufferObject
{
    
    public:
        
        OpenGLES2Texture();
        OpenGLES2Texture(const STextureCreationFlags &CreationFlags);
        ~OpenGLES2Texture();
        
        /* Functions */
        
        void updateImageBuffer();
        
    private:
        
        friend class GLProgrammableFunctionPipeline;
        friend class GLFramebufferObject;
        
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
