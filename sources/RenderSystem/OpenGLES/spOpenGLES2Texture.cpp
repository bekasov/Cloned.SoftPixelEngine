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

OpenGLES2Texture::OpenGLES2Texture() :
    GLTextureBase(),
    GLFrameBufferObject()
{
}
OpenGLES2Texture::OpenGLES2Texture(const STextureCreationFlags &CreationFlags) :
    GLTextureBase(CreationFlags),
    GLFrameBufferObject()
{
    updateFormatAndDimension();
    updateImageBuffer();
}
OpenGLES2Texture::~OpenGLES2Texture()
{
    deleteFramebuffer();
}

bool OpenGLES2Texture::updateImageBuffer()
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
    
    /* Update render target (create/delete) */
    deleteFramebuffer();
    
    if (isRenderTarget_)
    {
        createFramebuffer(
            getTexID(), ImageBuffer_->getSize(), GLDimension_, ImageBuffer_->getFormat(),
            DimensionType_, CubeMapFace_, ArrayLayer_,
            DepthBufferSource_ ? static_cast<OpenGLES2Texture*>(DepthBufferSource_)->DepthBufferID_ : 0
        );
    }
    
    return true;
}


/*
 * ======= Private: =======
 */

void OpenGLES2Texture::updateFormatAndDimension()
{
    /* Update OpenGL format, internal format and dimension */
    updateHardwareFormats();
    
    GLDimension_ = GLBasePipeline::getGlTexDimension(DimensionType_);
}

void OpenGLES2Texture::updateHardwareFormats()
{
    /* Get GL format */
    const EPixelFormats Format = ImageBuffer_->getFormat();
    
    if (Format >= PIXELFORMAT_ALPHA && Format <= PIXELFORMAT_DEPTH)
    {
        GLFormat_            = GLTexInternalFormatListUByte8[Format];
        GLInternalFormat_    = GLTexInternalFormatListUByte8[Format];
    }
}

void OpenGLES2Texture::updateHardwareTexture(
    dim::vector3di Size, const u32 PixelSize, const void* ImageBuffer, s32 Level)
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
                GL_TEXTURE_2D, Level, GLInternalFormat_, Size.X, Size.Y,
                0, GLFormat_, GL_UNSIGNED_BYTE, ImageBuffer
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
            const u32 OffsetSize = Size.X*Size.Y*PixelSize;
            const s8* Buffer = static_cast<const s8*>(ImageBuffer);
            
            for (s32 i = 0; i < 6; ++i)
            {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Level, GLInternalFormat_, Size.X, Size.Y,
                    0, GLFormat_, GL_UNSIGNED_BYTE, Buffer
                );
                
                if (Buffer)
                    Buffer += OffsetSize;
            }
            
            if (MipMaps_)
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }
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
