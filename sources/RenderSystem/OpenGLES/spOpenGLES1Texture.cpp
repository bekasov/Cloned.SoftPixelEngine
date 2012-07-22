/*
 * OpenGL|ES 1 texture header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGLES/spOpenGLES1Texture.hpp"

#if defined(SP_COMPILE_WITH_OPENGLES1)


#include "Base/spImageManagement.hpp"
//#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#include "RenderSystem/OpenGLES/spOpenGLES1RenderSystem.hpp"


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

OpenGLES1Texture::OpenGLES1Texture() : GLTextureBase()
{
}
OpenGLES1Texture::OpenGLES1Texture(const STextureCreationFlags &CreationFlags)
    : GLTextureBase(CreationFlags)
{
    updateFormat();
    
    if (CreationFlags.ImageBuffer)
        Texture::updateImageBuffer(CreationFlags.ImageBuffer);
}
OpenGLES1Texture::~OpenGLES1Texture()
{
}

void OpenGLES1Texture::updateImageBuffer()
{
    /* Check if texture needs to be recreated */
    if (GLDimension_ != GLFixedFunctionPipeline::getGlTexDimension(DimensionType_))
        updateFormat();
    
    /* Clear the image data */
    glBindTexture(GLDimension_, getTexID());
    
    /* Update format and texture image */
    updateTextureAttributes();
    updateTextureImage();
}


/*
 * ======= Private: =======
 */

void OpenGLES1Texture::updateFormat()
{
    /* Update OpenGL format, internal format and dimension */
    setupTextureFormats(Format_, HWFormat_, GLFormat_, GLInternalFormat_);
    
    updateFormatSize();
    
    GLDimension_ = GLFixedFunctionPipeline::getGlTexDimension(DimensionType_);
}

void OpenGLES1Texture::setupTextureFormats(
    const EPixelFormats Format, const EHWTextureFormats HWFormat, GLenum &GLFormat, GLenum &GLInternalFormat)
{
    if (Format >= PIXELFORMAT_INDEX && Format <= PIXELFORMAT_BGRA)
    {
        GLFormat            = GLTexInternalFormatListUByte8[Format];
        GLInternalFormat    = GLTexInternalFormatListUByte8[Format];
    }
}

void OpenGLES1Texture::updateTextureImageNormal(
    dim::vector3di Size, s32 FormatSize, GLenum GLInternalFormat, GLenum GLFormat, const u8* ImageBuffer, s32 Level)
{
    static const io::stringc NotSupported = "textures are not supported for OpenGL|ES 1";
    
    switch (DimensionType_)
    {
        case TEXTURE_1D:
            io::Log::error("1D " + NotSupported);
            break;
            
        case TEXTURE_2D:
            glTexImage2D(
                GL_TEXTURE_2D, Level, GLInternalFormat, Size.X, Size.Y,
                0, GLFormat, GL_UNSIGNED_BYTE, ImageBuffer
            );
            break;
            
        case TEXTURE_3D:
            io::Log::error("3D " + NotSupported);
            break;
            
        case TEXTURE_CUBEMAP:
            io::Log::error("CubeMap " + NotSupported);
            break;
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
