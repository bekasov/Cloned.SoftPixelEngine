/*
 * Direct3D11 shader class header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D11_SHADERCLASS_H__
#define __SP_DIRECT3D11_SHADERCLASS_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/spShaderProgram.hpp"

#include <d3d11.h>
#include <d3d11Shader.h>
#include <d3dcompiler.h>
#include <map>


namespace sp
{
namespace video
{


class SP_EXPORT Direct3D11ShaderClass : public ShaderClass
{
    
    public:
        
        Direct3D11ShaderClass(const VertexFormat* VertexInputLayout = 0);
        ~Direct3D11ShaderClass();
        
        /* === Functions === */

        void bind(const scene::MaterialNode* Object = 0);
        void unbind();
        
        bool link();
        
    private:
        
        friend class Direct3D11Shader;
        friend class Direct3D11RenderSystem;
        
        /* === Members === */
        
        ID3D11DeviceContext* D3DDeviceContext_;
        
        ID3D11VertexShader*     VertexShaderObject_;
        ID3D11PixelShader*      PixelShaderObject_;
        ID3D11GeometryShader*   GeometryShaderObject_;
        ID3D11HullShader*       HullShaderObject_;
        ID3D11DomainShader*     DomainShaderObject_;
        ID3D11ComputeShader*    ComputeShaderObject_;
        
        std::vector<ID3D11Buffer*>* VertexConstantBuffers_;
        std::vector<ID3D11Buffer*>* PixelConstantBuffers_;
        std::vector<ID3D11Buffer*>* GeometryConstantBuffers_;
        std::vector<ID3D11Buffer*>* HullConstantBuffers_;
        std::vector<ID3D11Buffer*>* DomainConstantBuffers_;
        std::vector<ID3D11Buffer*>* ComputeConstantBuffers_;
        
        ID3D11InputLayout* InputVertexLayout_;
        const VertexFormat* VertexFormat_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
