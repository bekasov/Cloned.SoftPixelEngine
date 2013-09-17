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
#include "RenderSystem/OpenGL/spOpenGLConstantBuffer.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace video
{


OpenGLShader::OpenGLShader(
    ShaderClass* ShdClass, const EShaderTypes Type, const EShaderVersions Version) :
    Shader              (ShdClass, Type, Version),
    ProgramObject_      (0                      ),
    ShaderObject_       (0                      ),
    AsmShaderProgram_   (0                      ),
    AsmShaderType_      (0                      )
{
    ProgramObject_ = static_cast<OpenGLShaderClass*>(ShdClass_)->ProgramObject_;
    
    updateShaderClass();
}
OpenGLShader::~OpenGLShader()
{
    #ifdef SP_COMPILE_WITH_OPENGL
    if (isHighLevel())
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
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint, const c8** CompilerOptions, u32 Flags)
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

const SShaderConstant* OpenGLShader::getConstantRef(const io::stringc &Name) const
{
    for (u32 i = 0, c = ConstantList_.size(); i < c; ++i)
    {
        if (ConstantList_[i].Name == Name || ConstantList_[i].AltName == Name)
            return (&ConstantList_[i]);
    }
    return 0;
}

const SShaderConstant& OpenGLShader::getConstant(const io::stringc &Name) const
{
    for (u32 i = 0, c = ConstantList_.size(); i < c; ++i)
    {
        if (ConstantList_[i].Name == Name || ConstantList_[i].AltName == Name)
            return ConstantList_[i];
    }
    return Shader::EmptyConstant_;
}


/* === Index-based constant functions === */

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


/* === String-based constant functions === */

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
    const SShaderConstant* Constant = getConstantRef(Name);
    return Constant ? setConstant(*Constant, Buffer, Count) : false;
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

bool OpenGLShader::setConstant(const io::stringc &Name, const dim::vector3df &Vector)
{
    const SShaderConstant* Constant = getConstantRef(Name);
    return Constant ? setConstant(*Constant, Vector) : false;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const dim::vector4df &Vector)
{
    const SShaderConstant* Constant = getConstantRef(Name);
    return Constant ? setConstant(*Constant, Vector) : false;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const video::color &Color)
{
    const SShaderConstant* Constant = getConstantRef(Name);
    return Constant ? setConstant(*Constant, Color) : false;
}

bool OpenGLShader::setConstant(const io::stringc &Name, const dim::matrix4f &Matrix)
{
    const SShaderConstant* Constant = getConstantRef(Name);
    return Constant ? setConstant(*Constant, Matrix) : false;
}


/* === Structure-based constnat functions === */

bool OpenGLShader::setConstant(const SShaderConstant &Constant, const f32 Value)
{
    return setConstant(Constant.Name, Value);
}

bool OpenGLShader::setConstant(const SShaderConstant &Constant, const f32* Buffer, s32 Count)
{
    /* Check parameter validity */
    if (!Constant.valid())
        return false;
    
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    /* Select the type */
    switch (Constant.Type)
    {
        case CONSTANT_FLOAT:
            glUniform1fvARB(Constant.Location, Count, Buffer);
            break;
        case CONSTANT_VECTOR2:
            glUniform2fvARB(Constant.Location, Count/2, Buffer);
            break;
        case CONSTANT_VECTOR3:
            glUniform3fvARB(Constant.Location, Count/3, Buffer);
            break;
        case CONSTANT_VECTOR4:
            glUniform4fvARB(Constant.Location, Count/4, Buffer);
            break;
        case CONSTANT_MATRIX2:
            glUniformMatrix2fvARB(Constant.Location, Count/4, GL_FALSE, Buffer);
            break;
        case CONSTANT_MATRIX3:
            glUniformMatrix3fvARB(Constant.Location, Count/9, GL_FALSE, Buffer);
            break;
        case CONSTANT_MATRIX4:
            glUniformMatrix4fvARB(Constant.Location, Count/16, GL_FALSE, Buffer);
            break;
        default:
            glUniform1ivARB(Constant.Location, Count, reinterpret_cast<const GLint*>(Buffer));
            break;
    }
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}

bool OpenGLShader::setConstant(const SShaderConstant &Constant, const s32 Value)
{
    return setConstant(Constant.Name, Value);
}

bool OpenGLShader::setConstant(const SShaderConstant &Constant, const s32* Buffer, s32 Count)
{
    return setConstant(Constant.Name, Buffer, Count);
}

bool OpenGLShader::setConstant(const SShaderConstant &Constant, const dim::vector3df &Vector)
{
    /* Check parameter validity */
    if (!Constant.valid())
        return false;
    
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    bool Result = true;
    
    /* Select the type */
    switch (Constant.Type)
    {
        case CONSTANT_VECTOR3:
            glUniform3fARB(Constant.Location, Vector.X, Vector.Y, Vector.Z);
            break;
        case CONSTANT_VECTOR4:
            glUniform4fARB(Constant.Location, Vector.X, Vector.Y, Vector.Z, 1.0f);
            break;
        default:
            Result = false;
            break;
    }
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return Result;
}

bool OpenGLShader::setConstant(const SShaderConstant &Constant, const dim::vector4df &Vector)
{
    /* Check parameter validity */
    if (!Constant.valid())
        return false;
    
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    bool Result = true;
    
    /* Select the type */
    switch (Constant.Type)
    {
        case CONSTANT_VECTOR3:
            glUniform3fARB(Constant.Location, Vector.X, Vector.Y, Vector.Z);
            break;
        case CONSTANT_VECTOR4:
            glUniform4fARB(Constant.Location, Vector.X, Vector.Y, Vector.Z, Vector.W);
            break;
        default:
            Result = false;
            break;
    }
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return Result;
}

bool OpenGLShader::setConstant(const SShaderConstant &Constant, const video::color &Color)
{
    /* Check parameter validity */
    if (!Constant.valid())
        return false;
    
    /* Use the current shader program */
    glUseProgramObjectARB(ProgramObject_);
    
    bool Result = true;
    
    /* Select the type */
    switch (Constant.Type)
    {
        case CONSTANT_VECTOR3:
            glUniform3fARB(
                Constant.Location,
                static_cast<f32>(Color.Red  ) / 255.0f,
                static_cast<f32>(Color.Green) / 255.0f,
                static_cast<f32>(Color.Blue ) / 255.0f
            );
            break;
        case CONSTANT_VECTOR4:
            glUniform4fARB(
                Constant.Location,
                static_cast<f32>(Color.Red  ) / 255.0f,
                static_cast<f32>(Color.Green) / 255.0f,
                static_cast<f32>(Color.Blue ) / 255.0f,
                static_cast<f32>(Color.Alpha) / 255.0f
            );
            break;
        default:
            Result = false;
            break;
    }
    
    /* Unbind the current shader program */
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return Result;
}

bool OpenGLShader::setConstant(const SShaderConstant &Constant, const dim::matrix4f &Matrix)
{
    /* Check parameter validity */
    if (!Constant.valid() || Constant.Type != CONSTANT_MATRIX4)
        return false;
    
    /* Set shader constant */
    glUseProgramObjectARB(ProgramObject_);
    glUniformMatrix4fvARB(Constant.Location, 1, false, Matrix.getArray());
    glUseProgramObjectARB(OpenGLShaderClass::LastProgramObject_);
    
    return true;
}


/* === Other constant functions === */

bool OpenGLShader::setConstant(const f32* Buffer, s32 StartRegister, s32 ConstAmount)
{
    #ifdef SP_COMPILE_WITH_OPENGL
    GLenum ShaderTarget = (Type_ == SHADER_PIXEL_PROGRAM ? GL_FRAGMENT_PROGRAM_ARB : GL_VERTEX_PROGRAM_ARB);

    for (s32 i = 0; i < ConstAmount; ++i)
        glProgramLocalParameter4fvARB(ShaderTarget, StartRegister + i, &Buffer[i*4]);

    return true;
    #else
    return false;
    #endif
}

bool OpenGLShader::setConstantBuffer(const io::stringc &Name, const void* Buffer)
{
    /* Get shader constant buffer by name */
    ConstantBuffer* ConstBuffer = getConstantBuffer(Name);
    
    if (!ConstBuffer)
        return false;
    
    /* Update constant buffer data */
    if (!ConstBuffer->updateBuffer(Buffer))
        return false;
    
    /* Setup constant buffer binding point */
    const GLuint BlockIndex = ConstBuffer->getIndex();
    
    glBindBufferBase(GL_UNIFORM_BUFFER, BlockIndex, static_cast<OpenGLConstantBuffer*>(ConstBuffer)->getBufferID());
    glUniformBlockBinding(ProgramObject_, BlockIndex, BlockIndex);
    
    return true;
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
    GLenum ShaderType = 0;
    
    switch (Type_)
    {
        /* Basic shaders */
        case SHADER_VERTEX:
            ShaderType = GL_VERTEX_SHADER; break;
        case SHADER_PIXEL:
            ShaderType = GL_FRAGMENT_SHADER; break;
        
        #ifdef SP_COMPILE_WITH_OPENGL
        /* Geometry shader */
        case SHADER_GEOMETRY:
            ShaderType = GL_GEOMETRY_SHADER; break;
        
        #   ifdef GL_ARB_tessellation_shader
        /* Tessellation shaders */
        case SHADER_HULL:
            ShaderType = GL_TESS_CONTROL_SHADER; break;
        case SHADER_DOMAIN:
            ShaderType = GL_TESS_EVALUATION_SHADER; break;
        #   endif
        
        #   ifdef GL_ARB_compute_shader
        /* Compute shaders */
        case SHADER_COMPUTE:
            ShaderType = GL_COMPUTE_SHADER; break;
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

void OpenGLShader::addShaderConstant(const c8* Name, const GLenum Type, u32 Count, s32 Location)
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
    
    /* Setup basic members */
    Constant.Name       = io::stringc(Name);
    Constant.Count      = Count;
    Constant.Location   = Location;
    
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
