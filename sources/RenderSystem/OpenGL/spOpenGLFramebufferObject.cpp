/*
 * OpenGL framebuffer object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLFramebufferObject.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/OpenGL/spOpenGLPipelineBase.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)
#   include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#   include "RenderSystem/OpenGL/spOpenGLTexture.hpp"
#elif defined(SP_COMPILE_WITH_OPENGLES2)
#   include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#   include "RenderSystem/OpenGLES/spOpenGLES2Texture.hpp"
#endif

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


GLFramebufferObject::GLFramebufferObject() :
    FrameBufferID_  (0),
    DepthBufferID_  (0),
    MSFrameBufferID_(0)
{
}
GLFramebufferObject::~GLFramebufferObject()
{
}


/*
 * ======= Protected: =======
 */

void GLFramebufferObject::createFramebuffer(
    GLuint TexID, const dim::size2di &Size, GLenum GLDimension, const EPixelFormats Format,
    const ETextureDimensions DimensionType, const ECubeMapDirections CubeMapFace,
    u32 ArrayLayer, GLuint DepthBufferSourceID)
{
    if (!__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET] || !TexID)
        return;
    
    /* Create depth buffer */
    if (!DepthBufferSourceID)
    {
        glGenRenderbuffersEXT(1, &DepthBufferID_);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, DepthBufferID_);
        
        #ifdef SP_COMPILE_WITH_OPENGL
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, Size.Width, Size.Height);
        #else
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, Size.Width, Size.Height);
        #endif
        
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
        
        DepthBufferSourceID = DepthBufferID_;
    }
    
    /* Create frame buffer */
    glGenFramebuffersEXT(1, &FrameBufferID_);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FrameBufferID_);
    
    glFramebufferRenderbufferEXT(
        GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, DepthBufferSourceID
    );
    
    /* Attach frame buffer texture */
    attachFramebufferTexture(TexID, GLDimension, Format, DimensionType, CubeMapFace, ArrayLayer);
    
    /* Unbind framebuffer */
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

#ifdef SP_COMPILE_WITH_OPENGL

void GLFramebufferObject::createFramebufferMultisample(
    GLuint TexID, const dim::size2di &Size, GLenum GLDimension, GLenum GLInternalFormat,
    s32 Samples, const std::vector<Texture*> &MultiRenderTargets, const EPixelFormats Format,
    const ETextureDimensions DimensionType, const ECubeMapDirections CubeMapFace,
    u32 ArrayLayer, GLuint DepthBufferSourceID)
{
    if (!__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET] ||
        !__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_MULTISAMPLE_RENDERTARGET] || !TexID || Samples < 0)
    {
        return;
    }
    
    MSColorBufferID_.resize(MultiRenderTargets.size() + 1);
    
    math::Clamp(Samples, 0, GL_MAX_SAMPLES);
    
    /* Create multi-sample color buffer */
    if (!MSColorBufferID_.empty())
    {
        glGenRenderbuffersEXT(MSColorBufferID_.size(), &MSColorBufferID_[0]);
        
        for (u32 i = 0; i < MSColorBufferID_.size(); ++i)
        {
            if (i)
                GLInternalFormat = static_cast<GLTextureBase*>(MultiRenderTargets[i - 1])->GLInternalFormat_;
            
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, MSColorBufferID_[i]);
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, Samples, GLInternalFormat, Size.Width, Size.Height);
        }
    }
    
    /* Create multi-sample depth buffer */
    if (!DepthBufferSourceID)
    {
        glGenRenderbuffersEXT(1, &DepthBufferID_);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, DepthBufferID_);
        
        glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, Samples, GL_DEPTH_COMPONENT24, Size.Width, Size.Height);
        
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
        
        DepthBufferSourceID = DepthBufferID_;
    }
    
    /* Create multi-sample frame buffer */
    glGenFramebuffersEXT(1, &MSFrameBufferID_);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, MSFrameBufferID_);
    
    /* Bind color- and depth buffers */
    GLenum Attachment = GL_COLOR_ATTACHMENT0_EXT;
    foreach (GLuint Id, MSColorBufferID_)
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, Attachment++, GL_RENDERBUFFER_EXT, Id);
    
    glFramebufferRenderbufferEXT(
        GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, DepthBufferSourceID
    );
    
    /* Create frame buffer */
    glGenFramebuffersEXT(1, &FrameBufferID_);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FrameBufferID_);
    
    /* Attach frame buffer texture */
    attachFramebufferTexture(TexID, GLDimension, Format, DimensionType, CubeMapFace, ArrayLayer);
    
    /* Unbind framebuffer */
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void GLFramebufferObject::blitFramebufferMultisample(const dim::size2di &Size, u32 RTCount)
{
    if (!__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET] ||
        !__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_MULTISAMPLE_RENDERTARGET])
    {
        return;
    }
    
    /* Blit framebuffer */
    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, MSFrameBufferID_);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, FrameBufferID_);
    
    for (u32 i = 0; i <= RTCount; ++i)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
        glBlitFramebufferEXT(0, 0, Size.Width, Size.Height, 0, 0, Size.Width, Size.Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
    
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    /* Unbind multisample framebuffer objects */
    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
}

void GLFramebufferObject::updateMultiFramebuffer(
    const dim::size2di &Size, const EPixelFormats Format, const std::vector<Texture*> &MultiRenderTargets)
{
    s32 RTCount = MultiRenderTargets.size();
    
    if (!RTCount)
    {
        /* Disable multi-render-targets and only use the first color attachment */
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        return;
    }
    
    /* Determine maximal count of multi-render-targets */
    s32 MaxRTCount = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &MaxRTCount);
    
    if (RTCount > MaxRTCount - 1)
        RTCount = MaxRTCount - 1;
    
    /* Set the multi-render-targets */
    GLenum BufferIndex = GL_COLOR_ATTACHMENT0_EXT;
    std::vector<GLenum> MultiRTBuffers;
    
    /* Add first draw buffer */
    if (Format != PIXELFORMAT_DEPTH)
        MultiRTBuffers.push_back(BufferIndex++);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FrameBufferID_);
    
    for (s32 i = 0; i < RTCount; ++i)
    {
        Texture* Tex = MultiRenderTargets[i];
        
        if (Tex->getSize() != Size)
        {
            io::Log::error("MultiRenderTargets must have the same size");
            continue;
        }
        
        #if defined(SP_COMPILE_WITH_OPENGL)
        const GLenum GLDimension = static_cast<OpenGLTexture*>(Tex)->GLDimension_;
        #elif defined(SP_COMPILE_WITH_OPENGLES2)
        const GLenum GLDimension = static_cast<OpenGLES2Texture*>(Tex)->GLDimension_;
        #endif
        const GLuint TexID = *static_cast<GLuint*>(Tex->getID());
        
        if (MultiRenderTargets[i]->getFormat() != PIXELFORMAT_DEPTH)
        {
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, BufferIndex, GLDimension, TexID, 0);
            MultiRTBuffers.push_back(BufferIndex++);
        }
        else
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GLDimension, TexID, 0);
    }
    
    /* Activate draw buffers multi-render-targets */
    if (isMultiSampled())
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, MSFrameBufferID_);
    
    if (MultiRTBuffers.empty())
        glDrawBuffer(GL_NONE);
    else
        glDrawBuffersARB(MultiRTBuffers.size(), &MultiRTBuffers[0]);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

#endif

void GLFramebufferObject::setupCubeMapFace(
    GLuint TexID, const ECubeMapDirections CubeMapFace, bool isDepthAttachment, const std::vector<Texture*> &MultiRenderTargets)
{
    if (__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET])
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FrameBufferID_);
        
        /* Setup main render-target cubemap face */
        if (isDepthAttachment)
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + CubeMapFace, TexID, 0);
        else
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + CubeMapFace, TexID, 0);
        
        /* Setup multi-render-target cubemap faces */
        if (!MultiRenderTargets.empty())
        {
            GLenum Attachment = GL_COLOR_ATTACHMENT0_EXT;
            
            foreach (Texture* Tex, MultiRenderTargets)
            {
                TexID = *static_cast<GLuint*>(Tex->getID());
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, ++Attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + CubeMapFace, TexID, 0);
            }
        }
        
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
}

void GLFramebufferObject::setupArrayLayer(
    GLuint TexID, u32 Layer, bool isDepthAttachment, const std::vector<Texture*> &MultiRenderTargets)
{
    #ifdef SP_COMPILE_WITH_OPENGL
    
    if (__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET])
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FrameBufferID_);
        
        /* Setup main render-target layer */
        if (isDepthAttachment)
            glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, TexID, 0, Layer);
        else
            glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TexID, 0, Layer);
        
        /* Setup multi-render-target layers */
        if (!MultiRenderTargets.empty())
        {
            GLenum Attachment = GL_COLOR_ATTACHMENT0_EXT;
            
            foreach (Texture* Tex, MultiRenderTargets)
            {
                TexID = *static_cast<GLuint*>(Tex->getID());
                glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, ++Attachment, TexID, 0, Layer);
            }
        }
        
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
    
    #endif
}

void GLFramebufferObject::deleteFramebuffer()
{
    if (__spVideoDriver->RenderQuery_[RenderSystem::RENDERQUERY_RENDERTARGET])
    {
        /* Release frame- and render buffer */
        releaseFramebuffer(FrameBufferID_);
        releaseRenderbuffer(DepthBufferID_);
        
        /* Release multisample frame- and render buffer */
        releaseFramebuffer(MSFrameBufferID_);
        
        if (!MSColorBufferID_.empty())
        {
            glDeleteFramebuffersEXT(MSColorBufferID_.size(), &MSColorBufferID_[0]);
            MSColorBufferID_.clear();
        }
    }
}


/*
 * ======= Private: =======
 */

void GLFramebufferObject::releaseFramebuffer(GLuint &BufferID)
{
    if (BufferID)
    {
        glDeleteFramebuffersEXT(1, &BufferID);
        BufferID = 0;
    }
}
void GLFramebufferObject::releaseRenderbuffer(GLuint &BufferID)
{
    if (BufferID)
    {
        glDeleteRenderbuffersEXT(1, &BufferID);
        BufferID = 0;
    }
}

void GLFramebufferObject::attachFramebufferTexture(
    GLuint TexID, GLenum GLDimension, const EPixelFormats Format,
    const ETextureDimensions DimensionType, const ECubeMapDirections CubeMapFace, u32 ArrayLayer)
{
    /* Get texture target */
    const GLenum TexTarget = (DimensionType == TEXTURE_CUBEMAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + CubeMapFace : GLDimension);
    
    /* Attach texture to framebuffer */
    if (Format == PIXELFORMAT_DEPTH)
    {
        #ifdef SP_COMPILE_WITH_OPENGL
        glDrawBuffer(GL_NONE);
        #endif
        
        #ifdef SP_COMPILE_WITH_OPENGL
        if (DimensionType >= TEXTURE_1D_ARRAY)
            glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, TexID, 0, ArrayLayer);
        else
        #endif
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, TexTarget, TexID, 0);
    }
    else
    {
        #ifdef SP_COMPILE_WITH_OPENGL
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        #endif
        
        #ifdef SP_COMPILE_WITH_OPENGL
        if (DimensionType >= TEXTURE_1D_ARRAY)
            glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TexID, 0, ArrayLayer);
        else
        #endif
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TexTarget, TexID, 0);
    }
    
    /* Check for errors and unbind framebuffer */
    checkFrameBufferErrors();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
}

bool GLFramebufferObject::checkFrameBufferErrors()
{
    const GLenum Error = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    
    switch (Error)
    {
        case GL_FRAMEBUFFER_COMPLETE_EXT: // Successful
            return true;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            io::Log::error("FrameBufferObjects unsupported"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            io::Log::error("Incomplete attachment in FrameBufferObject"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            io::Log::error("Missing attachment in FrameBufferObject"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            io::Log::error("Incomplete dimensions in FrameBufferObject"); break;
        #ifdef SP_COMPILE_WITH_OPENGL
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            io::Log::error("Incomplete formats in FrameBufferObject"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            io::Log::error("Incomplete draw buffer in FrameBufferObject"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            io::Log::error("Incomplete read buffer in FrameBufferObject"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
            io::Log::error("Incomplete multisample buffer in FrameBufferObject"); break;
        #endif
    }
    
    return false;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
