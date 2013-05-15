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
#include "RenderSystem/OpenGL/spOpenGLFrameBufferObject.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT OpenGLES2Texture : public GLTextureBase, public GLFrameBufferObject
{
    
    public:
        
        OpenGLES2Texture();
        OpenGLES2Texture(const STextureCreationFlags &CreationFlags);
        ~OpenGLES2Texture();
        
        /* Functions */
        
        bool updateImageBuffer();
        
    private:
        
        friend class GLProgrammableFunctionPipeline;
        friend class GLFrameBufferObject;
        
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
