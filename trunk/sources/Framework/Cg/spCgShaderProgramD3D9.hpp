/*
 * Cg shader program D3D9 header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_CG_SHADERPROGRAM_D3D9_H__
#define __SP_CG_SHADERPROGRAM_D3D9_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_CG) && defined(SP_COMPILE_WITH_DIRECT3D9)


#include "Framework/Cg/spCgShaderProgram.hpp"
#include "RenderSystem/spShaderConfigTypes.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT CgShaderProgramD3D9 : public CgShaderProgram
{
    
    public:
        
        CgShaderProgramD3D9(ShaderTable* Table, const EShaderTypes Type, const EShaderVersions Version);
        ~CgShaderProgramD3D9();
        
    private:
        
        /* Functions */
        
        void bind();
        void unbind();
        
        bool compileCg(const io::stringc &SourceCodeString, const io::stringc &EntryPoint);
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
