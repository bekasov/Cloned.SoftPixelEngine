/*
 * OpenGL texture header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIDEODRIVER_OPENGLTEXTURE_H__
#define __SP_VIDEODRIVER_OPENGLTEXTURE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLTextureBase.hpp"
#include "RenderSystem/OpenGL/spOpenGLFramebufferObject.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT OpenGLTexture : public GLTextureBase, public GLFramebufferObject
{
    
    public:
        
        OpenGLTexture();
        OpenGLTexture(const STextureCreationFlags &CreationFlags);
        ~OpenGLTexture();
        
        /* Functions */
        
        void setCubeMapFace(const ECubeMapDirections Face);
        void setArrayLayer(u32 Layer);
        
        bool shareImageBuffer();
        bool updateImageBuffer();
        bool updateImageBuffer(const dim::point2di &Pos, const dim::size2di &Size);
        
    private:
        
        friend class GLProgrammableFunctionPipeline;
        friend class GLFramebufferObject;
        
        /* Private functions */
        
        void updateMultiRenderTargets();
        void updateFramebufferMultisample();
        
        void updateFormatAndDimension();
        
        void updateHardwareFormats();
        void updateHardwareTexture(
            dim::vector3di Size, const u32 PixelSize, const void* ImageBuffer, s32 Level = 0
        );
        void updateHardwareTextureArea(
            const dim::vector3di &Pos, const dim::vector3di &Size, const void* ImageBuffer, s32 Level = 0
        );
        
        void updateRenderTarget();
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
