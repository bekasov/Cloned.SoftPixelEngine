/*
 * OpenGL programmable function renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLPipelineProgrammable.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "SceneGraph/spSceneCamera.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)
#   include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#   include "RenderSystem/OpenGL/spOpenGLTexture.hpp"
#   include "RenderSystem/OpenGL/spOpenGLShaderResource.hpp"
#elif defined(SP_COMPILE_WITH_OPENGLES2)
#   include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#   include "RenderSystem/OpenGLES/spOpenGLES2Texture.hpp"
#endif

#include <boost/foreach.hpp>


namespace sp
{

extern io::InputControl* GlbInputCtrl;
extern scene::SceneGraph* GlbSceneGraph;

namespace video
{


/*
 * ======= GLProgrammableFunctionPipeline class =======
 */

bool GLProgrammableFunctionPipeline::EnableBalanceRT_ = true;

GLProgrammableFunctionPipeline::GLProgrammableFunctionPipeline() :
    RenderSystem    (RENDERER_DUMMY ),
    GLBasePipeline  (               )
{
}
GLProgrammableFunctionPipeline::~GLProgrammableFunctionPipeline()
{
}

io::stringc GLProgrammableFunctionPipeline::getShaderVersion() const
{
    return glGetString(GL_SHADING_LANGUAGE_VERSION);
}


/*
 * ======= Shader programs =======
 */

ShaderClass* GLProgrammableFunctionPipeline::createShaderClass(const VertexFormat* VertexInputLayout)
{
    if (!RenderQuery_[RENDERQUERY_SHADER])
        return 0;
    
    ShaderClass* NewShaderClass = new OpenGLShaderClass(VertexInputLayout);
    ShaderClassList_.push_back(NewShaderClass);
    
    return NewShaderClass;
}

Shader* GLProgrammableFunctionPipeline::createShader(
    ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    Shader* NewShader = 0;
    
    if (RenderQuery_[RENDERQUERY_SHADER])
        NewShader = new OpenGLShader(ShaderClassObj, Type, Version);
    else
        NewShader = new Shader(ShaderClassObj, Type, Version);
    
    NewShader->compile(ShaderBuffer);
    
    if (!ShaderClassObj)
        NewShader->getShaderClass()->link();
    
    ShaderList_.push_back(NewShader);
    
    return NewShader;
}

void GLProgrammableFunctionPipeline::unbindShaders()
{
    if (RenderQuery_[RENDERQUERY_SHADER])
    {
        /* Unbind the high level shader */
        OpenGLShaderClass::LastProgramObject_ = 0;
        glUseProgramObjectARB(0);
        
        #ifdef SP_COMPILE_WITH_OPENGL
        /* Unbind the assembler shader */
        glDisable(GL_VERTEX_PROGRAM_ARB);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
        #endif
    }
}

#ifdef SP_COMPILE_WITH_OPENGL

ShaderResource* GLProgrammableFunctionPipeline::createShaderResource()
{
    if (!RenderQuery_[RENDERQUERY_SHADER_RESOURCE])
    {
        io::Log::error("Shader resources are not available for this render system");
        return 0;
    }
    
    ShaderResource* NewResource = new OpenGLShaderResource();
    ShaderResourceList_.push_back(NewResource);
    
    return NewResource;
}

bool GLProgrammableFunctionPipeline::runComputeShader(ShaderClass* ShdClass, const dim::vector3d<u32> &GroupSize)
{
    if (!RenderQuery_[RENDERQUERY_COMPUTE_SHADER])
    {
        io::Log::error("Compute shaders are not supported by this render system");
        return false;
    }
    
    /* Check parameters for validity */
    video::Shader* ShaderObj = (ShdClass ? ShdClass->getComputeShader() : 0);
    
    if (!ShaderObj || ShaderObj->getType() != SHADER_COMPUTE || !ShaderObj->valid())
    {
        io::Log::error("Specified object is not a valid compute shader class");
        return false;
    }
    
    if (GroupSize.X < 1 || GroupSize.Y < 1 || GroupSize.Z < 1)
    {
        io::Log::error("Invalid thread group size for compute shader execution");
        return false;
    }

    /* Bind textures */
    //!TODO! -> use start slot to allow user to bind custom textures first
    s32 i = 0;
    foreach (Texture* Tex, ShdClass->getRWTextureList())
        Tex->bind(i++);
    
    /* Bind shader */
    ShdClass->bind();
    
    /* Dispatch the compute shader pipeline */
    glDispatchCompute(GroupSize.X, GroupSize.Y, GroupSize.Z);

    /* Unbind shader */
    ShdClass->unbind();
    
    /* Unind textures */
    i = 0;
    foreach (Texture* Tex, ShdClass->getRWTextureList())
        Tex->unbind(i++);
    
    return true;
}

#endif


/*
 * ======= Render targets =======
 */

bool GLProgrammableFunctionPipeline::setRenderTarget(Texture* Target)
{
    if (!RenderQuery_[RENDERQUERY_RENDERTARGET])
        return false;
    if (Target == RenderTarget_)
        return true;
    
    /* Update previous render target */
    if (RenderTarget_)
    {
        #ifdef SP_COMPILE_WITH_OPENGL
        /* Blit multisampled */
        if (RenderTarget_->getMultiSamples() > 0)
            static_cast<OpenGLTexture*>(RenderTarget_)->updateFramebufferMultisample();
        #endif
        
        /* Update mipmaps */
        RenderTarget_->generateMipMap();
    }
    
    if (Target)
    {
        if (Target->getRenderTarget())
        {
            /* Bind new render target */
            #if defined(SP_COMPILE_WITH_OPENGL)
            OpenGLTexture* GLTex = static_cast<OpenGLTexture*>(Target);
            #elif defined(SP_COMPILE_WITH_OPENGLES2)
            OpenGLES2Texture* GLTex = static_cast<OpenGLES2Texture*>(Target);
            #endif
            
            #if defined(SP_COMPILE_WITH_OPENGL)
            if (Target->getMultiSamples() > 0)
                glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, GLTex->MSFrameBufferID_);
            else
            #endif
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, GLTex->FrameBufferID_);
            
            RenderTarget_ = Target;
            
            if (EnableBalanceRT_)
                setInvertScreen(true);
        }
    }
    else if (RenderTarget_)
    {
        /* Undbind render target */
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        
        RenderTarget_ = 0;
        
        if (isInvertScreen_)
            setInvertScreen(false);
    }
    
    return true;
}


/* === Special renderer functions === */

void GLProgrammableFunctionPipeline::setBalanceRT(bool Enable)
{
    EnableBalanceRT_ = Enable;
}
bool GLProgrammableFunctionPipeline::getBalanceRT()
{
    return EnableBalanceRT_;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
