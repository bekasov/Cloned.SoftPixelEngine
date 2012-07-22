/*
 * OpenGL shader table file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLShaderTable.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "RenderSystem/OpenGL/spOpenGLShader.hpp"
#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


/*
 * OpenGL shader table class
 */

GLhandleARB OpenGLShaderTable::LastProgramObject_ = 0;

OpenGLShaderTable::OpenGLShaderTable(VertexFormat* VertexInputLayout)
    : ShaderTable(), ProgramObject_(0), VertexInputLayout_(VertexInputLayout)
{
    ProgramObject_ = glCreateProgramObjectARB();
    
    #ifdef SP_COMPILE_WITH_OPENGLES2
    HighLevel_ = true;
    #endif
}
OpenGLShaderTable::~OpenGLShaderTable()
{
    if (ProgramObject_)
        glDeleteProgram(ProgramObject_);
}

void OpenGLShaderTable::bind(const scene::MaterialNode* Object)
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

void OpenGLShaderTable::unbind()
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

bool OpenGLShaderTable::link()
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
        if ( checkLinkingErrors() || !setupUniforms() ||
             ( VertexShader_    && !VertexShader_   ->valid() ) ||
             ( PixelShader_     && !PixelShader_    ->valid() ) ||
             ( GeometryShader_  && !GeometryShader_ ->valid() ) ||
             ( HullShader_      && !HullShader_     ->valid() ) ||
             ( DomainShader_    && !DomainShader_   ->valid() ) )
        {
            CompiledSuccessfully_ = false;
        }
        
        // Delete the shader objects
        deleteShaderObject(VertexShader_);
        deleteShaderObject(PixelShader_);
    #ifdef SP_COMPILE_WITH_OPENGL
        deleteShaderObject(GeometryShader_);
        deleteShaderObject(HullShader_);
        deleteShaderObject(DomainShader_);
    }
    #endif
    
    return CompiledSuccessfully_;
}

void OpenGLShaderTable::deleteShaderObject(Shader* ShaderObject)
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

bool OpenGLShaderTable::checkLinkingErrors()
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

bool OpenGLShaderTable::setupUniforms()
{
    s32 Count = 0, MaxLen = 0;
    
    // Get the count of uniforms
    glGetProgramiv(ProgramObject_, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &Count);
    
    if (!Count)
        return true;
    
    // Get the maximal uniform length
    glGetProgramiv(ProgramObject_, GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, &MaxLen);
    
    if (!MaxLen)
        return false;
    
    c8* Name = new c8[MaxLen];
    GLsizei NameLen;
    GLenum Type;
    s32 Size;
    
    // Receive the uniform information
    for (s32 i = 0; i < Count; ++i)
    {
        // Get the active uniform
        glGetActiveUniformARB(ProgramObject_, i, MaxLen, &NameLen, &Size, &Type, Name);
        
        // Check for valid uniform name
        if ( !NameLen || ( NameLen >= 3 && Name[0] == 'g' && Name[1] == 'l' && Name[2] == '_' ) )
            continue;
        
        //!TODO! -> handle uniform arrays in a different way!
        // Remove array attachment "[0]"
        for (s32 j = NameLen - 1; j > 0; --j)
        {
            if (Name[j] == '[')
            {
                Name[j] = 0;
                break;
            }
        }
        
        // Add the element to the list
        addShaderConstant(Name, Type, Size);
    }
    
    delete [] Name;
    
    return true;
}

void OpenGLShaderTable::setupVertexFormat(VertexFormat* VertexInputLayout)
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

void OpenGLShaderTable::addShaderConstant(const c8* Name, const GLenum Type, u32 Count)
{
    // Add uniform to all shaders
    if (VertexShader_)
        static_cast<OpenGLShader*>(VertexShader_)->addShaderConstant(Name, Type, Count);
    if (PixelShader_)
        static_cast<OpenGLShader*>(PixelShader_)->addShaderConstant(Name, Type, Count);
    #ifdef SP_COMPILE_WITH_OPENGL
    if (GeometryShader_)
        static_cast<OpenGLShader*>(GeometryShader_)->addShaderConstant(Name, Type, Count);
    if (HullShader_)
        static_cast<OpenGLShader*>(HullShader_)->addShaderConstant(Name, Type, Count);
    if (DomainShader_)
        static_cast<OpenGLShader*>(DomainShader_)->addShaderConstant(Name, Type, Count);
    #endif
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
