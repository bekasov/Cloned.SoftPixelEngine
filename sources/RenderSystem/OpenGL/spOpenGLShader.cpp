/*
 * OpenGL shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLShader.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace video
{


OpenGLShader::OpenGLShader(
    ShaderClass* Table, const EShaderTypes Type, const EShaderVersions Version) :
    Shader              (Table, Type, Version   ),
    ShaderObject_       (0                      ),
    AsmShaderProgram_   (0                      ),
    AsmShaderType_      (0                      )
{
    if (!ShdClass_)
        ShdClass_ = new OpenGLShaderClass();
    
    ProgramObject_ = static_cast<OpenGLShaderClass*>(ShdClass_)->ProgramObject_;
    
    updateShaderClass();
}
OpenGLShader::~OpenGLShader()
{
    #ifdef SP_COMPILE_WITH_OPENGL
    if (HighLevel_)
    {
    #endif
        if (ShaderObject_)
            glDeleteShader(ShaderObject_);
    #ifdef SP_COMPILE_WITH_OPENGL
    }
    else if (AsmShaderProgram_)
        glDeleteProgramsARB(1, &AsmShaderProgram_);
    #endif
}

bool OpenGLShader::compile(
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint, const c8** CompilerOptions)
{
    bool Result = false;
    
    #ifdef SP_COMPILE_WITH_OPENGL
    if (Type_ == SHADER_VERTEX_PROGRAM)
    {
        AsmShaderType_ = GL_VERTEX_PROGRAM_ARB;
        Result = compileProgram(ShaderBuffer);
    }
    else if (Type_ == SHADER_PIXEL_PROGRAM)
    {
        AsmShaderType_ = GL_FRAGMENT_PROGRAM_ARB;
        Result = compileProgram(ShaderBuffer);
    }
    else
    #endif
        Result = compileGLSL(ShaderBuffer);
    
    return CompiledSuccessfully_ = Result;
}


/* Set the constants (by number) */

bool OpenGLShader::setConstant(s32 Number, const EConstantTypes Type, const f32 Float)
{
    return setConstant(Number, Type, &Float, 1);
}

bool OpenGLShader::setConstant(s32 Number, const EConstantTypes Type, const f32* Buffer, s32 Count)
{
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    /* Select the type */
    switch (Type)
    {
        case CONSTANT_FLOAT:
            glUniform1fvARB(Number, Count, Buffer);
            break;
        case CONSTANT_VECTOR2:
            glUniform2fvARB(Number, Count/2, Buffer);
            break;
        case CONSTANT_VECTOR3:
            glUniform3fvARB(Number, Count/3, Buffer);
            break;
        case CONSTANT_VECTOR4:
            glUniform4fvARB(Number, Count/4, Buffer);
            break;
        case CONSTANT_MATRIX2:
            glUniformMatrix2fvARB(Number, Count/4, GL_FALSE, Buffer);
            break;
        case CONSTANT_MATRIX3:
            glUniformMatrix3fvARB(Number, Count/9, GL_FALSE, Buffer);
            break;
        case CONSTANT_MATRIX4:
            glUniformMatrix4fvARB(Number, Count/16, GL_FALSE, Buffer);
            break;
        default:
            glUniform1ivARB(Number, Count, (GLint*)Buffer);
            break;
    }
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(s32 Number, const EConstantTypes Type, const dim::vector3df &Position)
{
    return setConstant(Number, Type, &Position.X, 3);
}

bool OpenGLShader::setConstant(s32 Number, const EConstantTypes Type, const video::color &Color)
{
    f32 Buffer[4];
    Color.getFloatArray(Buffer);
    return setConstant(Number, Type, Buffer, 4);
}

bool OpenGLShader::setConstant(s32 Number, const EConstantTypes Type, const dim::matrix4f &Matrix)
{
    return setConstant(Number, Type, Matrix.getArray(), 16);
}

/* Set the constants (by name) */

bool OpenGLShader::setConstant(const io::stringc &Name, const f32 Value)
{
    /* Get uniform location */
    const s32 Location = glGetUniformLocationARB(ProgramObject_, Name.c_str());
    
    if (Location == -1)
        return false;
    
    /* Set shader constant */
    glUseProgramObjectARB(ProgramObject_);
    glUniform1fARB(Location, Value);
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const f32* Buffer, s32 Count)
{
    u32 i, c = ConstantList_.size();
    
    /* Loop all uniforms */
    for (i = 0; i < c; ++i)
    {
        if (ConstantList_[i].Name == Name || ConstantList_[i].AltName == Name)
            break;
    }
    
    if (i == c)
        return false;
    
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    const s32 Location = glGetUniformLocationARB(ProgramObject_, Name.c_str());
    
    if (Location == -1)
        return false;
    
    /* Select the type */
    switch (ConstantList_[i].Type)
    {
        case CONSTANT_FLOAT:
            glUniform1fvARB(Location, Count, Buffer);
            break;
        case CONSTANT_VECTOR2:
            glUniform2fvARB(Location, Count/2, Buffer);
            break;
        case CONSTANT_VECTOR3:
            glUniform3fvARB(Location, Count/3, Buffer);
            break;
        case CONSTANT_VECTOR4:
            glUniform4fvARB(Location, Count/4, Buffer);
            break;
        case CONSTANT_MATRIX2:
            glUniformMatrix2fvARB(Location, Count/4, GL_FALSE, Buffer);
            break;
        case CONSTANT_MATRIX3:
            glUniformMatrix3fvARB(Location, Count/9, GL_FALSE, Buffer);
            break;
        case CONSTANT_MATRIX4:
            glUniformMatrix4fvARB(Location, Count/16, GL_FALSE, Buffer);
            break;
        default:
            glUniform1ivARB(Location, Count, (GLint*)Buffer);
            break;
    }
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}


bool OpenGLShader::setConstant(const io::stringc &Name, const s32 Value)
{
    /* Get uniform location */
    const s32 Location = glGetUniformLocationARB(ProgramObject_, Name.c_str());
    
    if (Location == -1)
        return false;
    
    /* Set shader constant */
    glUseProgramObjectARB(ProgramObject_);
    glUniform1iARB(Location, Value);
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const s32* Buffer, s32 Count)
{
    /* Get uniform location */
    const s32 Location = glGetUniformLocationARB(ProgramObject_, Name.c_str());
    
    if (Location == -1)
        return false;
    
    /* Set shader constant */
    glUseProgramObjectARB(ProgramObject_);
    glUniform1ivARB(Location, Count, Buffer);
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const dim::vector3df &Position)
{
    u32 i, c = ConstantList_.size();
    
    /* Loop all uniforms */
    for (i = 0; i < c; ++i)
    {
        if (ConstantList_[i].Name == Name)
            break;
    }
    
    if (i == c)
        return false;
    
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    const s32 Location = glGetUniformLocationARB(ProgramObject_, Name.c_str());
    
    if (Location == -1)
        return false;
    
    /* Select the type */
    if (ConstantList_[i].Type == CONSTANT_VECTOR3)
        glUniform3fARB(Location, Position.X, Position.Y, Position.Z);
    else if (ConstantList_[i].Type == CONSTANT_VECTOR4)
        glUniform4fARB(Location, Position.X, Position.Y, Position.Z, 1.0f);
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const dim::vector4df &Position)
{
    u32 i, c = ConstantList_.size();
    
    /* Loop all uniforms */
    for (i = 0; i < c; ++i)
    {
        if (ConstantList_[i].Name == Name)
            break;
    }
    
    if (i == c)
        return false;
    
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    const s32 Location = glGetUniformLocationARB(ProgramObject_, Name.c_str());
    
    if (Location == -1)
        return false;
    
    /* Select the type */
    if (ConstantList_[i].Type == CONSTANT_VECTOR3)
        glUniform3fARB(Location, Position.X, Position.Y, Position.Z);
    else if (ConstantList_[i].Type == CONSTANT_VECTOR4)
        glUniform4fARB(Location, Position.X, Position.Y, Position.Z, Position.W);
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const video::color &Color)
{
    u32 i, c = ConstantList_.size();
    
    /* Loop all uniforms */
    for (i = 0; i < c; ++i)
    {
        if (ConstantList_[i].Name == Name)
            break;
    }
    
    if (i == c)
        return false;
    
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    const s32 Location = glGetUniformLocationARB(ProgramObject_, Name.c_str());
    
    if (Location == -1)
        return false;
    
    /* Select the type */
    if (ConstantList_[i].Type == CONSTANT_VECTOR3)
    {
        glUniform3fARB(
            Location,
            static_cast<f32>(Color.Red  ) / 255.0f,
            static_cast<f32>(Color.Green) / 255.0f,
            static_cast<f32>(Color.Blue ) / 255.0f
        );
    }
    else if (ConstantList_[i].Type == CONSTANT_VECTOR4)
    {
        glUniform4fARB(
            Location,
            static_cast<f32>(Color.Red  ) / 255.0f,
            static_cast<f32>(Color.Green) / 255.0f,
            static_cast<f32>(Color.Blue ) / 255.0f,
            static_cast<f32>(Color.Alpha) / 255.0f
        );
    }
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const dim::matrix4f &Matrix)
{
    /* Get uniform location */
    u32 i, c = ConstantList_.size();
    
    for (i = 0; i < c; ++i)
    {
        if (ConstantList_[i].Name == Name)
            break;
    }
    
    if (i == c || ConstantList_[i].Type != CONSTANT_MATRIX4)
        return false;
    
    /* Get uniform location */
    const s32 Location = glGetUniformLocationARB(ProgramObject_, Name.c_str());
    
    if (Location == -1)
        return false;
    
    /* Set shader constant */
    glUseProgramObjectARB(ProgramObject_);
    glUniformMatrix4fvARB(Location, 1, false, Matrix.getArray());
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(const f32* Buffer, s32 StartRegister, s32 ConstAmount)
{
    #ifdef SP_COMPILE_WITH_OPENGL
    for (s32 i = 0; i < ConstAmount; ++i)
        glProgramLocalParameter4fvARB(CurShaderProgram_, StartRegister + i, &Buffer[i*4]);
    return true;
    #else
    return false;
    #endif
}


/*
 * ======= Private: =======
 */

bool OpenGLShader::compileGLSL(const std::list<io::stringc> &ShaderBuffer)
{
    /* Create shader source strings */
    std::vector<const c8*> ShaderStrings(ShaderBuffer.size());
    
    u32 i = 0;
    foreach (const io::stringc &Str, ShaderBuffer)
        ShaderStrings[i++] = Str.c_str();
    
    if (ShaderStrings.empty())
    {
        io::Log::error("Can not compile empty GLSL shader");
        return false;
    }
    
    /* Create the shader object */
    GLenum ShaderType;
    
    switch (Type_)
    {
        case SHADER_VERTEX:
            ShaderType = GL_VERTEX_SHADER_ARB; break;
        case SHADER_PIXEL:
            ShaderType = GL_FRAGMENT_SHADER_ARB; break;
        #ifdef SP_COMPILE_WITH_OPENGL
        case SHADER_GEOMETRY:
            ShaderType = GL_GEOMETRY_SHADER_ARB; break;
        #   ifdef GL_ARB_tessellation_shader
        case SHADER_HULL:
            ShaderType = GL_TESS_CONTROL_SHADER; break;
        case SHADER_DOMAIN:
            ShaderType = GL_TESS_EVALUATION_SHADER; break;
        #   endif
        #endif
        default:
            return false;
    }
    
    ShaderObject_ = glCreateShaderObjectARB(ShaderType);
    
    /* Initialize the data */
    glShaderSourceARB(ShaderObject_, ShaderStrings.size(), &ShaderStrings[0], 0);
    
    /* Compile shader */
    glCompileShaderARB(ShaderObject_);
    
    /* Check for compiling errors */
    if (checkCompilingErrors())
        return false;
    
    /* Attach object */
    glAttachObjectARB(ProgramObject_, ShaderObject_);
    
    #ifdef SP_COMPILE_WITH_OPENGL
    
    /* Geometry shader configuration */
    if (Type_ == SHADER_GEOMETRY)
    {
        glProgramParameteriEXT(ProgramObject_, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
        glProgramParameteriEXT(ProgramObject_, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLES);
        
        s32 MaxVerticesOutput;
        glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &MaxVerticesOutput);
        glProgramParameteriEXT(ProgramObject_, GL_GEOMETRY_VERTICES_OUT_EXT, MaxVerticesOutput);
    }
    
    #endif
    
    /* Link program */
    if (OwnShaderClass_)
        ShdClass_->link();
    
    return true;
}

#ifdef SP_COMPILE_WITH_OPENGL

bool OpenGLShader::compileProgram(const std::list<io::stringc> &ShaderBuffer)
{
    /* Convert the data into a single string */
    io::stringc ProgramString;
    
    foreach (const io::stringc &Line, ShaderBuffer)
        ProgramString += Line;
    
    /* Generate a new shader program */
    glGenProgramsARB(1, &AsmShaderProgram_);
    
    /* Bind the shader program */
    glBindProgramARB(AsmShaderType_, AsmShaderProgram_);
    
    /* Set the assemly shader string */
    glProgramStringARB(AsmShaderType_, GL_PROGRAM_FORMAT_ASCII_ARB, ProgramString.size(), ProgramString.c_str());
    
    /* Check for errors */
    if (checkCompilingErrors(AsmShaderType_ == GL_VERTEX_PROGRAM_ARB ? "Vertex" : "Fragment"))
    {
        /* Delete the shader program */
        glDeleteProgramsARB(1, &AsmShaderProgram_);
        AsmShaderProgram_ = 0;
        return false;
    }
    
    return true;
}

bool OpenGLShader::checkCompilingErrors(const io::stringc &ShaderName)
{
    CompiledSuccessfully_ = (glGetError() == GL_NO_ERROR);
    
    if (!CompiledSuccessfully_)
    {
        s32 ErrorPos;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &ErrorPos);
        
        const c8* ErrorStr = (const c8*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        
        printError(
            ShaderName + " shader compilation failed at position " +
            io::stringc(ErrorPos) + ":\n" + io::stringc(ErrorStr)
        );
        
        return true;
    }
    
    return false;
}

#endif

bool OpenGLShader::checkCompilingErrors()
{
    // Get the compilation status
    s32 CompileStatus = 0;
    glGetShaderiv(ShaderObject_, GL_OBJECT_COMPILE_STATUS_ARB, &CompileStatus);
    
    // Get the error log information length
    s32 LogLength = 0;
    glGetShaderiv(ShaderObject_, GL_OBJECT_INFO_LOG_LENGTH_ARB, &LogLength);
    
    if (LogLength > 1)
    {
        s32 CharsWritten = 0;
        c8* InfoLog = new c8[LogLength];
        
        // Get the error log information
        glGetShaderInfoLog(ShaderObject_, LogLength, &CharsWritten, InfoLog);
        
        if (CompileStatus == GL_TRUE)
            printWarning(InfoLog);
        else
            printError(InfoLog);
        
        delete [] InfoLog;
    }
    
    return (CompileStatus == GL_FALSE);
}

void OpenGLShader::addShaderConstant(const c8* Name, const GLenum Type, u32 Count)
{
    SShaderConstant Constant;
    
    /* Get type information */
    switch (Type)
    {
        case GL_BOOL:
            Constant.Type = CONSTANT_BOOL; break;
        case GL_INT:
        case GL_UNSIGNED_INT:
        #if defined(SP_COMPILE_WITH_OPENGL)
        case GL_SAMPLER_1D:
        case GL_SAMPLER_3D:
        #endif
        case GL_SAMPLER_2D:
        case GL_SAMPLER_CUBE:
            Constant.Type = CONSTANT_INT; break;
        case GL_FLOAT:
            Constant.Type = CONSTANT_FLOAT; break;
        case GL_FLOAT_VEC2_ARB:
            Constant.Type = CONSTANT_VECTOR2; break;
        case GL_FLOAT_VEC3_ARB:
            Constant.Type = CONSTANT_VECTOR3; break;
        case GL_FLOAT_VEC4_ARB:
            Constant.Type = CONSTANT_VECTOR4; break;
        case GL_FLOAT_MAT2_ARB:
            Constant.Type = CONSTANT_MATRIX2; break;
        case GL_FLOAT_MAT3_ARB:
            Constant.Type = CONSTANT_MATRIX3; break;
        case GL_FLOAT_MAT4_ARB:
            Constant.Type = CONSTANT_MATRIX4; break;
    }
    
    Constant.Name   = io::stringc(Name);
    Constant.Count  = Count;
    
    /* Check for alternative uniform-array name */
    if (Count > 1 && Constant.Name.rightEqual("[0]", 3))
    {
        Constant.AltName = Constant.Name;
        Constant.AltName.resize(Constant.AltName.size() - 3);
    }
    
    ConstantList_.push_back(Constant);
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
