/*
 * OpenGL texture header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLTexture.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "Base/spImageManagement.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#include "RenderSystem/OpenGL/spOpenGLRenderSystem.hpp"

#include <boost/shared_array.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


/*
 * Internal members
 */

// -> Don't use GL_DEPTH_COMPONENT16 here!
const GLenum GLTexFormatList[] = {
    GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_DEPTH_COMPONENT
};

const GLenum GLTexInternalFormatListFloat16[] = {
    GL_R16F, GL_LUMINANCE16F_ARB, GL_LUMINANCE_ALPHA16F_ARB, GL_RGB16F_ARB, GL_RGB16F_ARB, GL_RGBA16F_ARB, GL_RGBA16F_ARB, GL_DEPTH_COMPONENT24
};

const GLenum GLTexInternalFormatListFloat32[] = {
    GL_R32F, GL_LUMINANCE32F_ARB, GL_LUMINANCE_ALPHA32F_ARB, GL_RGB32F_ARB, GL_RGB32F_ARB, GL_RGBA32F_ARB, GL_RGBA32F_ARB, GL_DEPTH_COMPONENT32F
};

extern GLenum GLTexInternalFormatListUByte8[];


/*
 * OpenGLTexture class
 */

OpenGLTexture::OpenGLTexture() :
    GLTextureBase       (),
    GLFrameBufferObject ()
{
}
OpenGLTexture::OpenGLTexture(const STextureCreationFlags &CreationFlags) :
    GLTextureBase       (CreationFlags  ),
    GLFrameBufferObject (               )
{
    updateFormatAndDimension();
    updateImageBuffer();
}
OpenGLTexture::~OpenGLTexture()
{
    deleteFramebuffer();
}

void OpenGLTexture::setCubeMapFace(const ECubeMapDirections Face)
{
    Texture::setCubeMapFace(Face);
    if (isRenderTarget_ && DimensionType_ == TEXTURE_CUBEMAP)
    {
        setupCubeMapFace(
            getTexID(), CubeMapFace_, ImageBuffer_->getFormat() == PIXELFORMAT_DEPTH, MultiRenderTargetList_
        );
    }
}

void OpenGLTexture::setArrayLayer(u32 Layer)
{
    if ( ( DimensionType_ == TEXTURE_1D_ARRAY && Layer < static_cast<u32>(ImageBuffer_->getSize().Height) ) ||
         Layer < ImageBuffer_->getDepth() )
    {
        Texture::setArrayLayer(Layer);
        if (isRenderTarget_ && DimensionType_ >= TEXTURE_1D_ARRAY)
        {
            setupArrayLayer(
                getTexID(), ArrayLayer_, ImageBuffer_->getFormat() == PIXELFORMAT_DEPTH, MultiRenderTargetList_
            );
        }
    }
    #ifdef SP_DEBUGMODE
    else if (DimensionType_ == TEXTURE_1D_ARRAY && Layer < static_cast<u32>(ImageBuffer_->getSize().Height))
        io::Log::debug("OpenGLTexture::setArrayLayer", "'Layer' index out of range for 1D texture array");
    else
        io::Log::debug("OpenGLTexture::setArrayLayer", "'Layer' index out of range");
    #endif
}


/* === Image buffer === */

bool OpenGLTexture::shareImageBuffer()
{
    if (!ImageBuffer_->getBuffer())
        return false;
    
    /* Bind this texture */
    glBindTexture(GLDimension_, getTexID());
    
    /* Get the dimensions */
    GLint GLInternalFormat;
    s32 Width, Height;
    EPixelFormats Format = PIXELFORMAT_RGB;
    
    glGetTexLevelParameteriv(GLDimension_, 0, GL_TEXTURE_WIDTH, &Width);
    glGetTexLevelParameteriv(GLDimension_, 0, GL_TEXTURE_HEIGHT, &Height);
    glGetTexLevelParameteriv(GLDimension_, 0, GL_TEXTURE_INTERNAL_FORMAT, &GLInternalFormat);
    
    /* Determine texture format */
    switch (GLInternalFormat)
    {
        case GL_LUMINANCE8:
            GLFormat_   = GL_LUMINANCE;
            Format      = PIXELFORMAT_GRAY;
            break;
            
        case GL_LUMINANCE8_ALPHA8:
            GLFormat_   = GL_LUMINANCE_ALPHA;
            Format      = PIXELFORMAT_GRAYALPHA;
            break;
            
        case GL_RGB:
        case GL_RGB8:
        case GL_RGB16F_ARB:
        case GL_RGB32F_ARB:
            GLFormat_   = GL_RGB;
            Format      = PIXELFORMAT_RGB;
            break;
            
        case GL_RGBA:
        case GL_RGBA8:
        case GL_RGBA16F_ARB:
        case GL_RGBA32F_ARB:
            GLFormat_   = GL_RGBA;
            Format      = PIXELFORMAT_RGBA;
            break;
            
        default:
            io::Log::error("Unexpected internal texture format");
            return false;
    }
    
    /* Check if the dimensions has changed */
    ImageBuffer_->setSize(dim::size2di(Width, Height));
    ImageBuffer_->setFormat(Format);
    
    /* Update renderer texture format */
    updateFormatAndDimension();
    
    /* Get the image data */
    if (DimensionType_ == TEXTURE_CUBEMAP)
    {
        const u32 OffsetSize = ImageBuffer_->getSize().getArea() * ImageBuffer_->getPixelSize();
        
        glGetTexImage(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + CubeMapFace_, 0, GLFormat_,
            GLType_, static_cast<s8*>(ImageBuffer_->getBuffer()) + OffsetSize * static_cast<s32>(CubeMapFace_)
        );
    }
    else
        glGetTexImage(GLDimension_, 0, GLFormat_, GLType_, ImageBuffer_->getBuffer());
    
    return true;
}

bool OpenGLTexture::updateImageBuffer()
{
    /* Update dimension and format */
    const bool ReCreateTexture = (GLDimension_ != GLBasePipeline::getGlTexDimension(DimensionType_));
    
    updateFormatAndDimension();
    
    if (ReCreateTexture)
        createHWTexture();
    
    /* Bind texture */
    glBindTexture(GLDimension_, getTexID());
    
    /* Update hardware texture settings */
    updateTextureAttributes();
    updateTextureImage();
    
    /* Update render target (create/delete) */
    if (__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET])
        updateRenderTarget();
    
    /* Unbind texture */
    glBindTexture(GLDimension_, 0);
    
    return true;
}

bool OpenGLTexture::updateImageBuffer(const dim::point2di &Pos, const dim::size2di &Size)
{
    if (!ImageBuffer_->getBuffer() ||
        Size.Width <= 0 || Size.Height <= 0 ||
        Pos.X < 0 || Pos.Y < 0 ||
        Pos.X > getSize().Width - Size.Width ||
        Pos.Y > getSize().Height * static_cast<s32>(ImageBuffer_->getDepth()) - Size.Height)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("OpenGLTexture::updateImageBuffer");
        #endif
        return false;
    }
    
    /* Get image buffer area */
    const u32 BufferSize = Size.getArea() * ImageBuffer_->getPixelSize();
    boost::shared_array<c8> Buffer = boost::shared_array<c8>(new c8[BufferSize]);
    
    ImageBuffer_->getBuffer(Buffer.get(), Pos, Size);
    
    /* Update hardware texture for specified area */
    glBindTexture(GLDimension_, getTexID());
    
    updateHardwareTextureArea(
        dim::vector3di(Pos.X, Pos.Y / ImageBuffer_->getDepth(), Pos.Y % ImageBuffer_->getDepth()),
        dim::vector3di(Size.Width, Size.Height, 1),
        Buffer.get()
    );
    
    glBindTexture(GLDimension_, 0);
    
    return true;
}


/*
 * ======= Private: =======
 */

void OpenGLTexture::updateFormatAndDimension()
{
    /* Update OpenGL texture formats */
    updateHardwareFormats();
    
    /* Determine which dimension is used */
    GLDimension_ = GLBasePipeline::getGlTexDimension(DimensionType_);
}

void OpenGLTexture::updateMultiRenderTargets()
{
    updateImageBuffer();
    updateMultiFramebuffer(ImageBuffer_->getSize(), ImageBuffer_->getFormat(), MultiRenderTargetList_);
}
void OpenGLTexture::updateFramebufferMultisample()
{
    blitFramebufferMultisample(ImageBuffer_->getSize(), MultiRenderTargetList_.size());
}

void OpenGLTexture::updateHardwareFormats()
{
    /* Get GL format */
    const EPixelFormats Format = ImageBuffer_->getFormat();
    
    GLFormat_ = GLTexFormatList[Format];
    
    /* Get GL internal format */
    switch (HWFormat_)
    {
        case HWTEXFORMAT_UBYTE8:
            GLInternalFormat_ = GLTexInternalFormatListUByte8[Format];
            break;
        case HWTEXFORMAT_FLOAT16:
            GLInternalFormat_ = GLTexInternalFormatListFloat16[Format];
            break;
        case HWTEXFORMAT_FLOAT32:
            GLInternalFormat_ = GLTexInternalFormatListFloat32[Format];
            break;
        default:
            GLInternalFormat_ = GLFormat_;
            break;
    }
    
    /* Get GL image data type */
    switch (ImageBuffer_->getType())
    {
        case IMAGEBUFFER_UBYTE:
            GLType_ = GL_UNSIGNED_BYTE;
            break;
        case IMAGEBUFFER_FLOAT:
            GLType_ = GL_FLOAT;
            break;
    }
}

void OpenGLTexture::updateHardwareTexture(
    dim::vector3di Size, const u32 PixelSize, const void* ImageBuffer, s32 Level)
{
    if (DimensionType_ != TEXTURE_BUFFER)
        TBO_.detachBuffer();
    
    switch (DimensionType_)
    {
        case TEXTURE_1D:
        {
            glTexImage1D(
                GL_TEXTURE_1D, Level, GLInternalFormat_, Size.X, 0, GLFormat_, GLType_, ImageBuffer
            );
        }
        break;
        
        case TEXTURE_1D_ARRAY:
        case TEXTURE_2D:
        case TEXTURE_RECTANGLE:
        {
            glTexImage2D(
                GLDimension_, Level, GLInternalFormat_, Size.X, Size.Y, 0, GLFormat_, GLType_, ImageBuffer
            );
        }
        break;
        
        case TEXTURE_2D_ARRAY:
        case TEXTURE_CUBEMAP_ARRAY:
        case TEXTURE_3D:
        {
            glTexImage3DEXT(
                GLDimension_, Level, GLInternalFormat_, Size.X, Size.Y, Size.Z, 0, GLFormat_, GLType_, ImageBuffer
            );
        }
        break;
        
        case TEXTURE_CUBEMAP:
        {
            const u32 OffsetSize = Size.X*Size.Y*PixelSize;
            const s8* Buffer = static_cast<const s8*>(ImageBuffer);
            
            for (s32 i = 0; i < 6; ++i)
            {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Level, GLInternalFormat_,
                    Size.X, Size.Y, 0, GLFormat_, GLType_, Buffer
                );
                
                if (Buffer)
                    Buffer += OffsetSize;
            }
        }
        break;
        
        case TEXTURE_BUFFER:
        {
            TBO_.attachBuffer(
                ImageBuffer, Size.X*Size.Y*Size.Z*PixelSize, getFormat(), getHardwareFormat()
            );
        }
        break;
        
        default:
            io::Log::error("Unsupported texture dimension type for the OpenGL render system");
            break;
    }
}

void OpenGLTexture::updateHardwareTextureArea(
    const dim::vector3di &Pos, const dim::vector3di &Size, const void* ImageBuffer, s32 Level)
{
    switch (DimensionType_)
    {
        case TEXTURE_1D:
        {
            glTexSubImage1D(
                GL_TEXTURE_1D, Level, Pos.X, Size.X, GLFormat_, GLType_, ImageBuffer
            );
        }
        break;
        
        case TEXTURE_1D_ARRAY:
        case TEXTURE_2D:
        case TEXTURE_RECTANGLE:
        {
            glTexSubImage2D(
                GLDimension_, Level, Pos.X, Pos.Y, Size.X, Size.Y, GLFormat_, GLType_, ImageBuffer
            );
        }
        break;
        
        case TEXTURE_2D_ARRAY:
        case TEXTURE_CUBEMAP_ARRAY:
        case TEXTURE_3D:
        {
            glTexSubImage3DEXT(
                GLDimension_, Level, Pos.X, Pos.Y, Pos.Z, Size.X, Size.Y, Size.Z, GLFormat_, GLType_, ImageBuffer
            );
        }
        break;
        
        case TEXTURE_CUBEMAP:
        {
            for (s32 i = 0; i < 6; ++i)
            {
                glTexSubImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Level,
                    Pos.X, Pos.Y, Size.X, Size.Y, GLFormat_, GLType_, ImageBuffer
                );
            }
        }
        break;
        
        case TEXTURE_BUFFER:
            // ignore that here
            break;
        
        default:
            io::Log::error("Unsupported texture dimension type for the OpenGL render system");
            break;
    }
}

void OpenGLTexture::updateRenderTarget()
{
    deleteFramebuffer();
    
    if (isRenderTarget_)
    {
        const GLuint DepthBufferSourceID = (DepthBufferSource_ ? static_cast<OpenGLTexture*>(DepthBufferSource_)->DepthBufferID_ : 0);
        
        /* Create GL frame buffer object */
        #ifdef SP_COMPILE_WITH_OPENGL
        if (MultiSamples_ > 0 && __spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_MULTISAMPLE_RENDERTARGET])
        {
            createFramebufferMultisample(
                getTexID(), ImageBuffer_->getSize(), GLDimension_, GLInternalFormat_,
                MultiSamples_, MultiRenderTargetList_, ImageBuffer_->getFormat(),
                DimensionType_, CubeMapFace_, ArrayLayer_, DepthBufferSourceID
            );
        }
        else
        #endif
        {
            createFramebuffer(
                getTexID(), ImageBuffer_->getSize(), GLDimension_, ImageBuffer_->getFormat(),
                DimensionType_, CubeMapFace_, ArrayLayer_, DepthBufferSourceID
            );
        }
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
