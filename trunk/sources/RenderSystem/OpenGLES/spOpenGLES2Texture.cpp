/*
 * OpenGL|ES 2 texture file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGLES/spOpenGLES2Texture.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES2)


#include "Base/spImageManagement.hpp"
//#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#include "RenderSystem/OpenGLES/spOpenGLES2RenderSystem.hpp"
#include "RenderSystem/OpenGL/spOpenGLPipelineFixed.hpp"


namespace sp
{
namespace video
{


/*
 * Internal members
 */

extern GLenum GLTexInternalFormatListUByte8[];


/*
 * OpenGLES1Texture class
 */

OpenGLES2Texture::OpenGLES2Texture()
    : GLTextureBase(), GLFramebufferObject()
{
}
OpenGLES2Texture::OpenGLES2Texture(const STextureCreationFlags &CreationFlags)
    : GLTextureBase(CreationFlags), GLFramebufferObject()
{
    updateFormat();
    
    if (CreationFlags.ImageBuffer)
        Texture::updateImageBuffer(CreationFlags.ImageBuffer);
}
OpenGLES2Texture::~OpenGLES2Texture()
{
}

void OpenGLES2Texture::updateImageBuffer()
{
    /* Check if texture needs to be recreated */
    if (GLDimension_ != GLBasePipeline::getGlTexDimension(DimensionType_))
        updateFormat();
    
    /* Clear the image data */
    glBindTexture(GLDimension_, getTexID());
    
    /* Update format and texture image */
    updateTextureAttributes();
    updateTextureImage();
    
    /* Update render target (create/delete) */
    deleteFramebuffer();
    
    if (isRenderTarget_)
    {
        createFramebuffer(
            getTexID(), dim::size2di(Size_.Width, Size_.Height / Depth_),
            GLDimension_, Format_, DimensionType_, CubeMapFace_,
            DepthBufferSource_ ? static_cast<OpenGLES2Texture*>(DepthBufferSource_)->DepthBufferID_ : 0
        );
    }
}


/*
 * ======= Private: =======
 */

void OpenGLES2Texture::updateFormat()
{
    /* Update OpenGL format, internal format and dimension */
    setupTextureFormats(Format_, HWFormat_, GLFormat_, GLInternalFormat_);
    
    updateFormatSize();
    
    GLDimension_ = GLBasePipeline::getGlTexDimension(DimensionType_);
}

void OpenGLES2Texture::setupTextureFormats(
    const EPixelFormats Format, const EHWTextureFormats HWFormat, GLenum &GLFormat, GLenum &GLInternalFormat)
{
    if (Format >= PIXELFORMAT_INDEX && Format <= PIXELFORMAT_BGRA)
    {
        GLFormat            = GLTexInternalFormatListUByte8[Format];
        GLInternalFormat    = GLTexInternalFormatListUByte8[Format];
    }
}

void OpenGLES2Texture::updateTextureImageNormal(
    dim::vector3di Size, s32 FormatSize, GLenum GLInternalFormat, GLenum GLFormat, const u8* ImageBuffer, s32 Level)
{
    static const io::stringc NotSupported = "textures are not supported for OpenGL|ES 2";
    
    switch (DimensionType_)
    {
        case TEXTURE_1D:
            io::Log::error("1D " + NotSupported);
            break;
            
        case TEXTURE_2D:
        {
            glTexImage2D(
                GL_TEXTURE_2D, Level, GLInternalFormat, Size.X, Size.Y,
                0, GLFormat, GL_UNSIGNED_BYTE, ImageBuffer
            );
            
            if (MipMaps_)
                glGenerateMipmap(GL_TEXTURE_2D);
        }
        break;
            
        case TEXTURE_3D:
            io::Log::error("3D " + NotSupported);
            break;
            
        case TEXTURE_CUBEMAP:
        {
            const s32 OffsetSize = Size.X*Size.Y*FormatSize;
            
            for (s32 i = 0; i < 6; ++i)
            {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Level, GLInternalFormat, Size.X, Size.Y,
                    0, GLFormat, GL_UNSIGNED_BYTE, (ImageBuffer ? (const u8*)(ImageBuffer + i*OffsetSize) : 0)
                );
            }
            
            if (MipMaps_)
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }
        break;
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
