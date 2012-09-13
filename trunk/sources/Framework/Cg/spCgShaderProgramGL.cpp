/*
 * Cg shader program GL file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Cg/spCgShaderProgramGL.hpp"

#if defined(SP_COMPILE_WITH_CG) && defined(SP_COMPILE_WITH_OPENGL)


#include "Framework/Cg/spCgShaderContext.hpp"

#include <Cg/cgGL.h>


namespace sp
{
namespace video
{


CgShaderProgramGL::CgShaderProgramGL(
    ShaderClass* Table, const EShaderTypes Type, const EShaderVersions Version) :
    CgShaderProgram(Table, Type, Version)
{
    CGGLenum ProfileClass;
    
    switch (Type)
    {
        case SHADER_VERTEX:
            ProfileClass = CG_GL_VERTEX; break;
        case SHADER_PIXEL:
            ProfileClass = CG_GL_FRAGMENT; break;
        case SHADER_GEOMETRY:
            ProfileClass = CG_GL_GEOMETRY; break;
        case SHADER_HULL:
            ProfileClass = CG_GL_TESSELLATION_CONTROL; break;
        case SHADER_DOMAIN:
            ProfileClass = CG_GL_TESSELLATION_EVALUATION; break;
        default:
            io::Log::error("Invalid shader type for Cg profile");
            return;
    }
    
    cgProfile_ = cgGLGetLatestProfile(ProfileClass);
}
CgShaderProgramGL::~CgShaderProgramGL()
{
}

bool CgShaderProgramGL::setConstant(const io::stringc &Name, const f32* Buffer, s32 Count)
{
    if (getParam(Name, true))
    {
        cgGLSetParameterArray1f(ActiveParam_, 0, Count, Buffer);
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void CgShaderProgramGL::bind()
{
    if (!ParameterMap_.empty())
        cgUpdateProgramParameters(cgProgram_);
    
    cgGLBindProgram(cgProgram_);
    cgGLEnableProfile(cgProfile_);
}
void CgShaderProgramGL::unbind()
{
    cgGLUnbindProgram(cgProfile_);
    cgGLDisableProfile(cgProfile_);
}

bool CgShaderProgramGL::compileCg(
    const io::stringc &SourceCodeString, const io::stringc &EntryPoint, const c8** CompilerOptions)
{
    if (!createProgram(SourceCodeString, EntryPoint, CompilerOptions ? CompilerOptions : cgGLGetOptimalOptions(cgProfile_)))
        return false;
    
    cgGLSetOptimalOptions(cgProfile_);
    cgGLLoadProgram(cgProgram_);
    
    return !CgShaderContext::checkForError("shader program loading");
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
