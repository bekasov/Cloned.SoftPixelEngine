/*
 * Cg shader class header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_CG_SHADERCLASS_H__
#define __SP_CG_SHADERCLASS_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_CG)


#include "RenderSystem/spShaderProgram.hpp"


namespace sp
{
namespace video
{


class CgShaderProgram;

class SP_EXPORT CgShaderClass : public ShaderClass
{
    
    public:
        
        CgShaderClass(VertexFormat* VertexInputLayout = 0);
        ~CgShaderClass();
        
        void bind(const scene::MaterialNode* Object = 0);
        void unbind();
        
        bool link();
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
