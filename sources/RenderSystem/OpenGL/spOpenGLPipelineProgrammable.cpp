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
#elif defined(SP_COMPILE_WITH_OPENGLES2)
#   include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#   include "RenderSystem/OpenGLES/spOpenGLES2Texture.hpp"
#endif


namespace sp
{

extern io::InputControl* __spInputControl;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


/*
 * ======= GLProgrammableFunctionPipeline class =======
 */

bool GLProgrammableFunctionPipeline::EnableBalanceRT_ = true;

GLProgrammableFunctionPipeline::GLProgrammableFunctionPipeline()
    : RenderSystem(RENDERER_DUMMY), GLBasePipeline()
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

ShaderTable* GLProgrammableFunctionPipeline::createShaderTable(VertexFormat* VertexInputLayout)
{
    ShaderTable* NewShaderTable = new OpenGLShaderTable(VertexInputLayout);
    ShaderTableList_.push_back(NewShaderTable);
    return NewShaderTable;
}

Shader* GLProgrammableFunctionPipeline::createShader(
    ShaderTable* ShaderTableObj, const EShaderTypes Type, const EShaderVersions Version,
    const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    Shader* NewShader = 0;
    
    if (RenderQuery_[RENDERQUERY_SHADER])
        NewShader = new OpenGLShader(ShaderTableObj, Type, Version);
    else
        NewShader = new Shader(ShaderTableObj, Type, Version);
    
    NewShader->compile(ShaderBuffer);
    
    if (!ShaderTableObj)
        NewShader->getShaderTable()->link();
    
    ShaderList_.push_back(NewShader);
    
    return NewShader;
}

void GLProgrammableFunctionPipeline::unbindShaders()
{
    if (RenderQuery_[RENDERQUERY_SHADER])
    {
        /* Unbind the high level shader */
        OpenGLShaderTable::LastProgramObject_ = 0;
        glUseProgramObjectARB(0);
        
        #ifdef SP_COMPILE_WITH_OPENGL
        /* Unbind the assembler shader */
        glDisable(GL_VERTEX_PROGRAM_ARB);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
        #endif
    }
}


/*
 * ======= Render targets =======
 */

bool GLProgrammableFunctionPipeline::setRenderTarget(Texture* Target)
{
    if (!RenderQuery_[RENDERQUERY_RENDERTARGET])
        return false;
    
    /* Update previous render target */
    if (RenderTarget_)
    {
        #if defined(SP_COMPILE_WITH_OPENGL)
        OpenGLTexture* GLTex = static_cast<OpenGLTexture*>(RenderTarget_);
        #elif defined(SP_COMPILE_WITH_OPENGLES2)
        OpenGLES2Texture* GLTex = static_cast<OpenGLES2Texture*>(RenderTarget_);
        #endif
        
        #ifdef SP_COMPILE_WITH_OPENGL
        /* Blit multisampled */
        if (RenderTarget_->getMultiSamples() > 0)
            GLTex->updateFramebufferMultisample();
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
