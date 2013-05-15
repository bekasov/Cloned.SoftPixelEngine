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

OpenGLES1Texture::OpenGLES1Texture() :
    GLTextureBase()
{
}
OpenGLES1Texture::OpenGLES1Texture(const STextureCreationFlags &CreationFlags) :
    GLTextureBase(CreationFlags)
{
    updateFormatAndDimension();
    updateImageBuffer();
}
OpenGLES1Texture::~OpenGLES1Texture()
{
}

bool OpenGLES1Texture::updateImageBuffer()
{
    /* Update dimension and format */
    const bool ReCreateTexture = (GLDimension_ != GLBasePipeline::getGlTexDimension(DimensionType_));
    
    updateFormatAndDimension();
    
    if (ReCreateTexture)
        createHWTexture();
    
    /* Clear the image data */
    glBindTexture(GLDimension_, getTexID());
    
    /* Update format and texture image */
    updateTextureAttributes();
    updateTextureImage();
    
    return true;
}


/*
 * ======= Private: =======
 */

void OpenGLES1Texture::updateFormatAndDimension()
{
    /* Update OpenGL format, internal format and dimension */
    updateHardwareFormats();
    
    GLDimension_ = GLBasePipeline::getGlTexDimension(DimensionType_);
}

void OpenGLES1Texture::updateHardwareFormats()
{
    /* Get GL format */
    const EPixelFormats Format = ImageBuffer_->getFormat();
    
    if (Format >= PIXELFORMAT_ALPHA && Format <= PIXELFORMAT_BGRA)
    {
        GLFormat_            = GLTexInternalFormatListUByte8[Format];
        GLInternalFormat_    = GLTexInternalFormatListUByte8[Format];
    }
}

void OpenGLES1Texture::updateHardwareTexture(
    dim::vector3di Size, const u32 PixelSize, const void* ImageBuffer, s32 Level)
{
    static const io::stringc NotSupported = "textures are not supported for OpenGL|ES 1";
    
    switch (DimensionType_)
    {
        case TEXTURE_1D:
            io::Log::error("1D " + NotSupported);
            break;

        case TEXTURE_2D:
            glTexImage2D(
                GL_TEXTURE_2D, Level, GLInternalFormat_, Size.X, Size.Y,
                0, GLFormat_, GL_UNSIGNED_BYTE, ImageBuffer
            );
            break;

        case TEXTURE_3D:
            io::Log::error("3D " + NotSupported);
            break;
        case TEXTURE_CUBEMAP:
            io::Log::error("CubeMap " + NotSupported);
            break;
        case TEXTURE_1D_ARRAY:
            io::Log::error("1D array " + NotSupported);
            break;
        case TEXTURE_2D_ARRAY:
            io::Log::error("2D array " + NotSupported);
            break;
        case TEXTURE_CUBEMAP_ARRAY:
            io::Log::error("CubeMap array " + NotSupported);
            break;
        case TEXTURE_RECTANGLE:
            io::Log::error("Rectangle " + NotSupported);
            break;
        case TEXTURE_BUFFER:
            io::Log::error("Buffer " + NotSupported);
            break;
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
