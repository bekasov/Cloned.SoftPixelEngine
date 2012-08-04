/*
 * Direct3D11 shader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D11_SHADER_H__
#define __SP_DIRECT3D11_SHADER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11ShaderClass.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT Direct3D11Shader : public Shader
{
    
    public:
        
        Direct3D11Shader(ShaderClass* Table, const EShaderTypes Type, const EShaderVersions Version);
        ~Direct3D11Shader();
        
        /* Shader compilation */
        
        bool compile(const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "");
        
        /* Set the constant buffer */
        
        bool setConstantBuffer(const io::stringc &Name, const void* Buffer);
        bool setConstantBuffer(u32 Number, const void* Buffer);
        
        /* Constant table */
        
        u32 getConstantCount() const;
        std::vector<io::stringc> getConstantList() const;
        
    private:
        
        friend class Direct3D11ShaderClass;
        friend class Direct3D11RenderSystem;
        
        /* Functions */
        
        bool compileHLSL(const c8* ProgramBuffer, const c8* EntryPoint, const c8* TargetName);
        
        void createProgramString(const std::vector<io::stringc> &ShaderBuffer, c8* &ProgramBuffer);
        
        bool createConstantBuffers();
        
        /* Inline functions */
        
        inline u32 getVersionIndex(const EShaderVersions Min, const EShaderVersions Max) const
        {
            return math::MinMax(Version_, Min, Max) - Min;
        }
        
        /* Members */
        
        ID3D11Device* Device_;
        ID3D11DeviceContext* DeviceContext_;
        
        ID3D11VertexShader* VertexShaderObject_;
        ID3D11PixelShader* PixelShaderObject_;
        ID3D11GeometryShader* GeometryShaderObject_;
        ID3D11HullShader* HullShaderObject_;
        ID3D11DomainShader* DomainShaderObject_;
        ID3D11ComputeShader* ComputeShaderObject_;
        
        std::vector<ID3D11Buffer*> ConstantBuffers_;
        std::map<std::string, ID3D11Buffer*> ConstantBufferNames_;
        
        ID3D11InputLayout* InputVertexLayout_;
        ID3D11ShaderReflection* ShaderReflection_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
