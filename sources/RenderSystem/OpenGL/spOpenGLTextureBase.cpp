/*
 * OpenGL texture base file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLTextureBase.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES1) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/OpenGL/spOpenGLPipelineBase.hpp"
#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


/*
 * Internal members
 */

#ifdef SP_COMPILE_WITH_OPENGL

GLenum GLTextureWrapModes[] = {
    GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE
};

GLenum GLTexInternalFormatListUByte8[] = {
    GL_ALPHA8, GL_LUMINANCE8, GL_LUMINANCE8_ALPHA8, GL_RGB8, GL_RGB8, GL_RGBA8, GL_RGBA8, GL_DEPTH_COMPONENT16
};

#else

GLenum GLTextureWrapModes[] = {
    GL_REPEAT, GL_REPEAT, GL_CLAMP_TO_EDGE
};

GLenum GLTexInternalFormatListUByte8[] = {
    GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGB, GL_RGBA, GL_RGBA, GL_DEPTH_COMPONENT16
};

#endif


/*
 * GLTextureBase class
 */

GLTextureBase::GLTextureBase() :
    Texture             (   ),
    GLFormat_           (0  ),
    GLInternalFormat_   (0  ),
    GLDimension_        (0  ),
    GLType_             (0  )
{
    createHWTexture();
}
GLTextureBase::GLTextureBase(const STextureCreationFlags &CreationFlags) :
    Texture             (CreationFlags  ),
    GLFormat_           (0              ),
    GLInternalFormat_   (0              ),
    GLDimension_        (0              ),
    GLType_             (0              )
{
    createHWTexture();
}
GLTextureBase::~GLTextureBase()
{
    deleteHWTexture();
}

bool GLTextureBase::valid() const
{
    return OrigID_ && glIsTexture(getTexID());
}

void GLTextureBase::setFilter(const ETextureFilters Filter)
{
    if (MagFilter_ != Filter || MinFilter_ != Filter)
    {
        Texture::setFilter(Filter);
        updateTextureAttributesImmediate();
    }
}
void GLTextureBase::setFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter)
{
    if (MagFilter_ != MagFilter || MinFilter_ != MinFilter)
    {
        Texture::setFilter(MagFilter, MinFilter);
        updateTextureAttributesImmediate();
    }
}
void GLTextureBase::setMagFilter(const ETextureFilters Filter)
{
    if (MagFilter_ != Filter)
    {
        Texture::setMagFilter(Filter);
        updateTextureAttributesImmediate();
    }
}
void GLTextureBase::setMinFilter(const ETextureFilters Filter)
{
    if (MinFilter_ != Filter)
    {
        Texture::setMinFilter(Filter);
        updateTextureAttributesImmediate();
    }
}

void GLTextureBase::setMipMapFilter(const ETextureMipMapFilters MipMapFilter)
{
    if (MipMapFilter_ != MipMapFilter)
    {
        Texture::setMipMapFilter(MipMapFilter);
        updateTextureAttributesImmediate();
    }
}

void GLTextureBase::setWrapMode(const ETextureWrapModes Wrap)
{
    if (WrapMode_.X != Wrap || WrapMode_.Y != Wrap || WrapMode_.Z != Wrap)
    {
        Texture::setWrapMode(Wrap);
        updateTextureAttributesImmediate();
    }
}
void GLTextureBase::setWrapMode(
    const ETextureWrapModes WrapU, const ETextureWrapModes WrapV, const ETextureWrapModes WrapW)
{
    if (WrapMode_.X != WrapU || WrapMode_.Y != WrapV || WrapMode_.Z != WrapW)
    {
        Texture::setWrapMode(WrapU, WrapV, WrapW);
        updateTextureAttributesImmediate();
    }
}

void GLTextureBase::generateMipMap()
{
    if (MipMaps_)
    {
        glBindTexture(GLDimension_, getTexID());
        glGenerateMipmapEXT(GLDimension_);
    }
}

void GLTextureBase::bind(s32 Level) const
{
    /* Setup texture layers */
    if (GlbRenderSys->RenderQuery_[RenderSystem::RENDERQUERY_MULTI_TEXTURE])
        glActiveTextureARB(GL_TEXTURE0 + Level);
    
    /* Bind and enable the texture */
    glBindTexture(GLDimension_, *static_cast<GLuint*>(ID_));
    
    if (GlbRenderSys->getRendererType() != RENDERER_OPENGLES2 && Type_ <= TEXTURE_CUBEMAP)
        glEnable(GLDimension_);
}

void GLTextureBase::unbind(s32 Level) const
{
    /* Setup texture layers */
    if (GlbRenderSys->RenderQuery_[RenderSystem::RENDERQUERY_MULTI_TEXTURE])
        glActiveTextureARB(GL_TEXTURE0 + Level);
    
    /* Unbind and disable the texture */
    glBindTexture(GLDimension_, 0);
    
    if (GlbRenderSys->getRendererType() != RENDERER_OPENGLES2 && Type_ <= TEXTURE_CUBEMAP)
        glDisable(GLDimension_);
}


/*
 * ======= Private: =======
 */

void GLTextureBase::createHWTexture()
{
    /* Delete old OpenGL hardware texture */
    deleteHWTexture();
    
    /* Allocate memory for the OpenGL texture name */
    void* PrevID = OrigID_;
    
    /* Update current ID */
    OrigID_ = new GLuint;
    
    if (ID_ == PrevID)
        ID_ = OrigID_;
    
    /* Create new OpenGL hardware texture */
    glGenTextures(1, getTexPtrID());
}
void GLTextureBase::deleteHWTexture()
{
    if (OrigID_)
    {
        /* Delete OpenGL hardware texture */
        if (glIsTexture(getTexID()))
            glDeleteTextures(1, getTexPtrID());
        
        /* Delete memory of the OpenGL texture name */
        delete static_cast<GLuint*>(OrigID_);
        
        if (ID_ == OrigID_)
            ID_ = 0;
        OrigID_ = 0;
    }
}

void GLTextureBase::updateTextureAttributesImmediate()
{
    glBindTexture(GLDimension_, getTexID());
    updateTextureAttributes();
    glBindTexture(GLDimension_, 0);
}

void GLTextureBase::updateTextureAttributes()
{
    if (Type_ == TEXTURE_BUFFER)
        return;
    
    /* Wrap modes (reapeat, mirror, clamp) */
    glTexParameteri(GLDimension_, GL_TEXTURE_WRAP_S, GLTextureWrapModes[WrapMode_.X]);
    glTexParameteri(GLDimension_, GL_TEXTURE_WRAP_T, GLTextureWrapModes[WrapMode_.Y]);
    
    #if defined(SP_COMPILE_WITH_OPENGL)
    glTexParameteri(GLDimension_, GL_TEXTURE_WRAP_R, GLTextureWrapModes[WrapMode_.Z]);
    #endif
    
    /* MIP-mapping */
    if (Type_ != TEXTURE_RECTANGLE)
    {
        #if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES1)
        #   if defined(SP_COMPILE_WITH_OPENGLES1)
        if (GlbRenderSys->getRendererType() == RENDERER_OPENGLES1)
        #   endif
            glTexParameteri(GLDimension_, GL_GENERATE_MIPMAP, MipMaps_ ? GL_TRUE : GL_FALSE);
        #endif
    }

    /* Anisotropy */
    if (MipMapFilter_ == FILTER_ANISOTROPIC)
        glTexParameteri(GLDimension_, GL_TEXTURE_MAX_ANISOTROPY_EXT, AnisotropicSamples_);
    
    /* Magnification filter */
    glTexParameteri(GLDimension_, GL_TEXTURE_MAG_FILTER, MagFilter_ == FILTER_SMOOTH ? GL_LINEAR : GL_NEAREST);
    
    /* Minification filter */
    if (MinFilter_ == FILTER_SMOOTH)
    {
        if (MipMaps_)
        {
            glTexParameteri(
                GLDimension_, GL_TEXTURE_MIN_FILTER,
                MipMapFilter_ == FILTER_BILINEAR ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR
            );
        }
        else
            glTexParameteri(GLDimension_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else if (MinFilter_ == FILTER_LINEAR)
    {
        if (MipMaps_)
        {
            glTexParameteri(
                GLDimension_, GL_TEXTURE_MIN_FILTER,
                MipMapFilter_ == FILTER_BILINEAR ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR
            );
        }
        else
            glTexParameteri(GLDimension_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

void GLTextureBase::updateTextureImage()
{
    /* Setup texture formats */
    updateHardwareFormats();
    
    /* Upload image buffer to graphics hardware */
    updateHardwareTexture(
        ImageBuffer_->getSizeVector(), ImageBuffer_->getPixelSize(), ImageBuffer_->getBuffer()
    );
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
