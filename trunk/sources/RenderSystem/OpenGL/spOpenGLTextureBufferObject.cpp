/*
 * OpenGL texture buffer object (TBO) header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLTextureBufferObject.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"


namespace sp
{
namespace video
{


static const GLenum GLTexBufInternalFormats8[] =
{
    GL_R8, GL_R8, GL_RG8, 0, 0, GL_RGBA8, GL_RGBA8, 0
};

static const GLenum GLTexBufInternalFormats16F[] =
{
    GL_R16F, GL_R16F, GL_RG16F, 0, 0, GL_RGBA16F, GL_RGBA16F, 0
};

static const GLenum GLTexBufInternalFormats32F[] =
{
    GL_R32F, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_RGBA32F, 0
};

static const GLenum GLTexBufInternalFormats32I[] =
{
    GL_R32I, GL_R32I, GL_RG32I, GL_RGB32I, GL_RGB32I, GL_RGBA32I, GL_RGBA32I, 0
};


GLTextureBufferObject::GLTextureBufferObject() :
    GLHardwareBuffer(GL_TEXTURE_BUFFER)
{
}
GLTextureBufferObject::~GLTextureBufferObject()
{
}

bool GLTextureBufferObject::attachBuffer(
    const void* Buffer, u32 Size, const EPixelFormats Format, const EHWTextureFormats DataType)
{
    /* Get internal format and check for validity */
    const GLenum InternalFormat = getInternalFormat(Format, DataType);

    if (!InternalFormat)
    {
        io::Log::error("Invalid internal format for texture buffer object (GL TBO)");
        return false;
    }

    /* Create buffer object */
    createBuffer();

    /* Setup buffer data from texture buffer */
    setupBuffer(Buffer, Size, HWBUFFER_STATIC);

    /* Attach buffer to texture */
    glTexBuffer(GL_TEXTURE_BUFFER, InternalFormat, getBufferID());

    return true;
}

void GLTextureBufferObject::detachBuffer()
{
    if (hasBuffer())
    {
        /* Delete buffer object */
        deleteBuffer();

        /* Detach buffer from texture */
        glTexBuffer(GL_TEXTURE_BUFFER, 0, 0);
    }
}


/*
 * ======= Private: =======
 */

GLenum GLTextureBufferObject::getInternalFormat(const EPixelFormats Format, const EHWTextureFormats DataType) const
{
    switch (DataType)
    {
        case HWTEXFORMAT_UBYTE8:
            return GLTexBufInternalFormats8[Format];
        case HWTEXFORMAT_FLOAT16:
            return GLTexBufInternalFormats16F[Format];
        case HWTEXFORMAT_FLOAT32:
            return GLTexBufInternalFormats32F[Format];
        case HWTEXFORMAT_INT32:
            return GLTexBufInternalFormats32I[Format];
        default:
            break;
    }
    return 0;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
