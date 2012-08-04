/*
 * Cg shader context header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_CG_SHADERCONTEXT_H__
#define __SP_CG_SHADERCONTEXT_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_CG)


#include "Base/spMaterialConfigTypes.hpp"
#include "Framework/Cg/spCgShaderProgram.hpp"

#include <Cg/cg.h>


namespace sp
{
namespace video
{


//! Class for the "NVIDIA Cg Shader Context".
class SP_EXPORT CgShaderContext
{
    
    public:
        
        CgShaderContext();
        ~CgShaderContext();
        
        /* Functions */
        
        io::stringc getVersion() const;
        
        static bool checkForError(const io::stringc &Situation = "");
        
    private:
        
        friend class CgShaderProgram;
        
        /* Members */
        
        static CGcontext cgContext_;
        static ERenderSystems RendererType_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
