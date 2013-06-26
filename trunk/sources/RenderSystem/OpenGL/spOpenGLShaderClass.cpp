/*
 * OpenGL shader class file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLShaderClass.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "RenderSystem/OpenGL/spOpenGLShader.hpp"
#include "RenderSystem/OpenGL/spOpenGLConstantBuffer.hpp"
#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


/*
 * OpenGL shader class class
 */

GLuint OpenGLShaderClass::LastProgramObject_ = 0;

OpenGLShaderClass::OpenGLShaderClass(const VertexFormat* VertexInputLayout) :
    ShaderClass         (                   ),
    ProgramObject_      (0                  ),
    VertexInputLayout_  (VertexInputLayout  )
{
    ProgramObject_ = glCreateProgramObjectARB();
    
    #ifdef SP_COMPILE_WITH_OPENGLES2
    HighLevel_ = true;
    #endif
}
OpenGLShaderClass::~OpenGLShaderClass()
{
    MemoryManager::deleteList(ConstBufferList_);

    if (ProgramObject_)
        glDeleteProgram(ProgramObject_);
}

void OpenGLShaderClass::bind(const scene::MaterialNode* Object)
{
    if (ObjectCallback_)
        ObjectCallback_(this, Object);
    __spVideoDriver->setSurfaceCallback(SurfaceCallback_);
    
    #ifdef SP_COMPILE_WITH_OPENGL
    if (HighLevel_)
    {
    #endif
        LastProgramObject_ = ProgramObject_;
        glUseProgramObjectARB(ProgramObject_);
    #ifdef SP_COMPILE_WITH_OPENGL
    }
    else
    {
        if (VertexShader_ && !VertexShader_->isHighLevel())
        {
            glBindProgramARB(GL_VERTEX_PROGRAM_ARB, static_cast<OpenGLShader*>(VertexShader_)->AsmShaderProgram_);
            glEnable(GL_VERTEX_PROGRAM_ARB);
        }
        if (PixelShader_ && !PixelShader_->isHighLevel())
        {
            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, static_cast<OpenGLShader*>(PixelShader_)->AsmShaderProgram_);
            glEnable(GL_FRAGMENT_PROGRAM_ARB);
        }
    }
    #endif
}

void OpenGLShaderClass::unbind()
{
    #ifdef SP_COMPILE_WITH_OPENGL
    if (HighLevel_)
    {
    #endif
        LastProgramObject_ = 0;
        glUseProgramObjectARB(0);
    #ifdef SP_COMPILE_WITH_OPENGL
    }
    else
    {
        glDisable(GL_VERTEX_PROGRAM_ARB);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
    }
    #endif
}

bool OpenGLShaderClass::link()
{
    CompiledSuccessfully_ = true;
    
    #ifdef SP_COMPILE_WITH_OPENGL
    if (HighLevel_)
    {
    #endif
        // Update vertex input layout
        if (VertexInputLayout_ && VertexShader_)
            setupVertexFormat(VertexInputLayout_);
        
        // Link the shaders to an executable shader program
        glLinkProgramARB(ProgramObject_);
        
        // Check for linking errors and setup uniforms
        if ( checkLinkingErrors() ||
             !setupUniforms() ||
             !setupUniformBlocks() ||
             ( VertexShader_    && !VertexShader_   ->valid() ) ||
             ( PixelShader_     && !PixelShader_    ->valid() ) ||
             ( GeometryShader_  && !GeometryShader_ ->valid() ) ||
             ( HullShader_      && !HullShader_     ->valid() ) ||
             ( DomainShader_    && !DomainShader_   ->valid() ) )
        {
            CompiledSuccessfully_ = false;
        }
        
        // Delete the shader objects
        //deleteShaderObject(VertexShader_);
        //deleteShaderObject(PixelShader_);
    #ifdef SP_COMPILE_WITH_OPENGL
        /*deleteShaderObject(GeometryShader_);
        deleteShaderObject(HullShader_);
        deleteShaderObject(DomainShader_);*/
    }
    #endif
    
    return CompiledSuccessfully_;
}

void OpenGLShaderClass::deleteShaderObject(Shader* ShaderObject)
{
    if (ShaderObject)
    {
        OpenGLShader* RendererShader = static_cast<OpenGLShader*>(ShaderObject);
        if (RendererShader->ShaderObject_)
        {
            glDeleteShader(RendererShader->ShaderObject_);
            RendererShader->ShaderObject_ = 0;
        }
    }
}

bool OpenGLShaderClass::checkLinkingErrors()
{
    // Get the linking status
    s32 LinkStatus = 0;
    glGetProgramiv(ProgramObject_, GL_OBJECT_LINK_STATUS_ARB, &LinkStatus);
    
    // Get the error log information length
    s32 LogLength = 0;
    glGetProgramiv(ProgramObject_, GL_OBJECT_INFO_LOG_LENGTH_ARB, &LogLength);
    
    if (LogLength > 1)
    {
        s32 CharsWritten = 0;
        c8* InfoLog = new c8[LogLength];
        
        // Get the error log information
        glGetProgramInfoLog(ProgramObject_, LogLength, &CharsWritten, InfoLog);
        
        if (LinkStatus == GL_TRUE)
            printWarning(InfoLog);
        else
            printError(InfoLog);
        
        delete [] InfoLog;
    }
    
    return (LinkStatus == GL_FALSE);
}

bool OpenGLShaderClass::setupUniforms()
{
    /* Get the count of uniforms */
    GLint Count = 0;
    
    glGetProgramiv(ProgramObject_, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &Count);
    
    if (!Count)
        return true;
    
    /* Get the maximal uniform length */
    GLint MaxLen = 0;

    glGetProgramiv(ProgramObject_, GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, &MaxLen);
    
    if (!MaxLen)
        return false;
    
    c8* Name = new c8[MaxLen];
    
    GLsizei NameLen = 0;
    GLsizei Size = 0;
    GLenum Type = 0;
    
    /* Receive the uniform information */
    for (GLint i = 0; i < Count; ++i)
    {
        /* Get the active uniform */
        glGetActiveUniformARB(ProgramObject_, i, MaxLen, &NameLen, &Size, &Type, Name);
        
        const s32 Location = glGetUniformLocationARB(ProgramObject_, Name);
        
        /* Add the element to the list */
        addShaderConstant(Name, Type, static_cast<u32>(Size), Location);
    }
    
    /* Clean up */
    delete [] Name;
    
    return true;
}

bool OpenGLShaderClass::setupUniformBlocks()
{
    /* Get the count of uniform blocks */
    GLint Count = 0;
    
    glGetProgramiv(ProgramObject_, GL_ACTIVE_UNIFORM_BLOCKS, &Count);
    
    if (!Count)
        return true;
    
    /* Get the maximal uniform block name length */
    GLint MaxLen = 0;
    
    glGetProgramiv(ProgramObject_, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &MaxLen);
    
    if (!MaxLen)
        return false;
    
    c8* Name = new c8[MaxLen];
    
    GLsizei NameLen = 0;
    bool Result = true;
    
    /* Receive the uniform block information */
    for (GLint i = 0; i < Count; ++i)
    {
        /* Get current uniform block name */
        NameLen = 0;
        
        glGetActiveUniformBlockName(ProgramObject_, i, MaxLen, &NameLen, Name);
        
        if (!NameLen)
        {
            io::Log::error("Problem with uniform block #" + io::stringc(i));
            Result = false;
            break;
        }
        
        /* Create new constant buffer */
        OpenGLConstantBuffer* ConstBuffer = new OpenGLConstantBuffer(this, io::stringc(Name), i);
        ConstBufferList_.push_back(ConstBuffer);
        
        /* Add constant buffer to shader */
        if (VertexShader_)
            static_cast<OpenGLShader*>(VertexShader_)->ConstantBufferList_.push_back(ConstBuffer);
        if (PixelShader_)
            static_cast<OpenGLShader*>(PixelShader_)->ConstantBufferList_.push_back(ConstBuffer);
    }
    
    /* Clean up */
    delete [] Name;
    
    return Result;
}

void OpenGLShaderClass::setupVertexFormat(const VertexFormat* VertexInputLayout)
{
    if (!VertexInputLayout || !VertexShader_)
        return;
    
    /* Bind vertex attribute locations */
    u32 i = 0;
    for (std::vector<SVertexAttribute>::const_iterator it = VertexInputLayout->getUniversals().begin();
         it != VertexInputLayout->getUniversals().end(); ++it, ++i)
    {
        if (i >= GL_MAX_VERTEX_ATTRIBS)
        {
            io::Log::error("Can not hold more than " + io::stringc(GL_MAX_VERTEX_ATTRIBS) + " attributes in OpenGL vertex shader");
            break;
        }
        if (it->Name.left(3) == "gl_")
        {
            io::Log::error("Invalid vertex attribute name: \"" + it->Name + "\" (must not start with \"gl_\") in " + VertexInputLayout->getIdentifier());
            continue;
        }
        
        glBindAttribLocationARB(
            static_cast<OpenGLShader*>(VertexShader_)->ProgramObject_, i, it->Name.c_str()
        );
    }
}

void OpenGLShaderClass::addShaderConstant(const c8* Name, const GLenum Type, u32 Count, s32 Location)
{
    // Add uniform to all shaders
    if (VertexShader_)
        static_cast<OpenGLShader*>(VertexShader_    )->addShaderConstant(Name, Type, Count, Location);
    if (PixelShader_)
        static_cast<OpenGLShader*>(PixelShader_     )->addShaderConstant(Name, Type, Count, Location);
    #ifdef SP_COMPILE_WITH_OPENGL
    if (GeometryShader_)
        static_cast<OpenGLShader*>(GeometryShader_  )->addShaderConstant(Name, Type, Count, Location);
    if (HullShader_)
        static_cast<OpenGLShader*>(HullShader_      )->addShaderConstant(Name, Type, Count, Location);
    if (DomainShader_)
        static_cast<OpenGLShader*>(DomainShader_    )->addShaderConstant(Name, Type, Count, Location);
    #endif
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
