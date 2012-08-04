/*
 * Cg shader program D3D9 file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Cg/spCgShaderProgramD3D9.hpp"

#if defined(SP_COMPILE_WITH_CG) && defined(SP_COMPILE_WITH_DIRECT3D9)


#include "Framework/Cg/spCgShaderContext.hpp"

#include <Cg/cgD3D9.h>


namespace sp
{
namespace video
{


CgShaderProgramD3D9::CgShaderProgramD3D9(
    ShaderClass* Table, const EShaderTypes Type, const EShaderVersions Version) :
    CgShaderProgram(Table, Type, Version)
{
    switch (Type)
    {
        case SHADER_VERTEX:
            cgProfile_ = cgD3D9GetLatestVertexProfile(); break;
        case SHADER_PIXEL:
            cgProfile_ = cgD3D9GetLatestPixelProfile(); break;
        default:
            io::Log::error("Invalid shader type for Cg profile");
            break;
    }
}
CgShaderProgramD3D9::~CgShaderProgramD3D9()
{
}


/*
 * ======= Private: =======
 */

void CgShaderProgramD3D9::bind()
{
    if (!ParameterMap_.empty())
        cgUpdateProgramParameters(cgProgram_);
    
    if (cgD3D9BindProgram(cgProgram_) != D3D_OK)
        io::Log::error("Could not bind Cg shader");
}
void CgShaderProgramD3D9::unbind()
{
    cgD3D9UnbindProgram(cgProgram_);
}

bool CgShaderProgramD3D9::compileCg(const io::stringc &SourceCodeString, const io::stringc &EntryPoint)
{
    if (!createProgram(SourceCodeString, EntryPoint, cgD3D9GetOptimalOptions(cgProfile_)))
        return false;
    
    cgD3D9LoadProgram(cgProgram_, false, 0);
    
    return !CgShaderContext::checkForError("shader program loading");
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
