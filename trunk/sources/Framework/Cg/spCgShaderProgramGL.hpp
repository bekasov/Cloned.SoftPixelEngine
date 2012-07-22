/*
 * Cg shader program GL header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_CG_SHADERPROGRAM_GL_H__
#define __SP_CG_SHADERPROGRAM_GL_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_CG) && defined(SP_COMPILE_WITH_OPENGL)


#include "Framework/Cg/spCgShaderProgram.hpp"
#include "RenderSystem/spShaderConfigTypes.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT CgShaderProgramGL : public CgShaderProgram
{
    
    public:
        
        CgShaderProgramGL(ShaderTable* Table, const EShaderTypes Type, const EShaderVersions Version);
        ~CgShaderProgramGL();
        
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
