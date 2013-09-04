/*
 * Direct3D9 shader class header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D9_SHADERCLASS_H__
#define __SP_DIRECT3D9_SHADERCLASS_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/spShaderProgram.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <d3d9.h>
#   include <d3dx9shader.h>
#endif


namespace sp
{
namespace video
{


class SP_EXPORT Direct3D9ShaderClass : public ShaderClass
{
    
    public:
        
        Direct3D9ShaderClass();
        ~Direct3D9ShaderClass();
        
        void bind(const scene::MaterialNode* Object = 0);
        void unbind();
        
        bool compile();
        
    private:
        
        /* Members */
        
        IDirect3DDevice9* D3DDevice_;
        
        IDirect3DVertexShader9* VertexShaderObject_;
        IDirect3DPixelShader9* PixelShaderObject_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
