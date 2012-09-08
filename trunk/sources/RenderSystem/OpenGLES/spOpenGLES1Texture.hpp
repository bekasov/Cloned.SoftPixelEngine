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
        
        bool updateImageBuffer();
        
    private:
        
        friend class OpenGLES1RenderSystem;
        
        /* Functions */
        
        void updateFormatAndDimension();
        void updateHardwareFormats();
        
        void updateHardwareTexture(
            dim::vector3di Size, const u32 PixelSize, const void* ImageBuffer, s32 Level = 0
        );
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
