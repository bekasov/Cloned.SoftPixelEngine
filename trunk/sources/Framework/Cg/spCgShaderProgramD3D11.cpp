/*
 * Cg shader program D3D11 file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Cg/spCgShaderProgramD3D11.hpp"

#if defined(SP_COMPILE_WITH_CG) && defined(SP_COMPILE_WITH_DIRECT3D11)


#include "Framework/Cg/spCgShaderContext.hpp"

#include <Cg/cgD3D11.h>


namespace sp
{
namespace video
{


CgShaderProgramD3D11::CgShaderProgramD3D11(ShaderTable* Table, const EShaderTypes Type, const EShaderVersions Version)
    : CgShaderProgram(Table, Type, Version)
{
    switch (Type)
    {
        case SHADER_VERTEX:
            cgProfile_ = cgD3D11GetLatestVertexProfile(); break;
        case SHADER_PIXEL:
            cgProfile_ = cgD3D11GetLatestPixelProfile(); break;
        case SHADER_GEOMETRY:
            cgProfile_ = cgD3D11GetLatestGeometryProfile(); break;
        case SHADER_HULL:
            cgProfile_ = cgD3D11GetLatestHullProfile(); break;
        case SHADER_DOMAIN:
            cgProfile_ = cgD3D11GetLatestDomainProfile(); break;
        default:
            io::Log::error("Invalid shader type for Cg profile");
            break;
    }
}
CgShaderProgramD3D11::~CgShaderProgramD3D11()
{
}


/*
 * ======= Private: =======
 */

void CgShaderProgramD3D11::bind()
{
    if (!ParameterMap_.empty())
        cgUpdateProgramParameters(cgProgram_);
    
    if (cgD3D11BindProgram(cgProgram_) != D3D_OK)
        io::Log::error("Could not bind Cg shader");
}
void CgShaderProgramD3D11::unbind()
{
    cgD3D11UnbindProgram(cgProgram_);
}

bool CgShaderProgramD3D11::compileCg(const io::stringc &SourceCodeString, const io::stringc &EntryPoint)
{
    if (!createProgram(SourceCodeString, EntryPoint, cgD3D11GetOptimalOptions(cgProfile_)))
        return false;
    
    cgD3D11LoadProgram(cgProgram_, 0);
    
    return !CgShaderContext::checkForError("shader program loading");
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
