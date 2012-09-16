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
    if (!Buffer)
        return false;
    
    /* Get top-level parameter */
    CGparameter Param = cgGetNamedParameter(cgProgram_, Name.c_str());
    
    if (!Param)
        return false;
    
    /* Get array parameter */
    if (cgGetParameterType(Param) != CG_ARRAY)
        return false;
    
    s32 ArraySize = cgGetArraySize(Param, 0);
    
    for (s32 i = 0; i < ArraySize; ++i)
    {
        /* Get array element parameter */
        CGparameter ElementParam = cgGetArrayParameter(Param, i);
        
        switch (cgGetParameterType(ElementParam))
        {
            case CG_FLOAT:
                cgGLSetParameterArray1f(Param, 0, Count, Buffer);
                return true;
            case CG_FLOAT2:
                cgGLSetParameterArray2f(Param, 0, Count/2, Buffer);
                return true;
            case CG_FLOAT3:
                cgGLSetParameterArray3f(Param, 0, Count/3, Buffer);
                return true;
            case CG_FLOAT4:
                cgGLSetParameterArray4f(Param, 0, Count/4, Buffer);
                return true;
            case CG_FLOAT4x4:
                cgGLSetMatrixParameterArrayfc(Param, 0, Count/16, Buffer);
                return true;
            
            case CG_STRUCT:
            {
                /* Get structure field parameter */
                CGparameter FieldParam = cgGetFirstStructParameter(ElementParam);
                
                while (FieldParam)
                {
                    switch (cgGetParameterType(FieldParam))
                    {
                        case CG_FLOAT:
                            cgGLSetParameter1f(FieldParam, *Buffer);
                            Buffer += 1;
                            break;
                        case CG_FLOAT2:
                            cgGLSetParameter2fv(FieldParam, Buffer);
                            Buffer += 2;
                            break;
                        case CG_FLOAT3:
                            cgGLSetParameter3fv(FieldParam, Buffer);
                            Buffer += 3;
                            break;
                        case CG_FLOAT4:
                            cgGLSetParameter4fv(FieldParam, Buffer);
                            Buffer += 4;
                            break;
                        case CG_FLOAT4x4:
                            cgGLSetMatrixParameterfc(FieldParam, Buffer);
                            Buffer += 16;
                            break;
                        case CG_INT:
                            cgSetParameter1i(FieldParam, *((s32*)Buffer));
                            Buffer += 1;
                            break;
                        default:
                            break;
                    }
                    
                    FieldParam = cgGetNextParameter(FieldParam);
                }
            }
            break;
            
            default:
                break;
        }
    }
    
    return true;
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
