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
        
        Direct3D11Shader(ShaderClass* ShdClass, const EShaderTypes Type, const EShaderVersions Version);
        ~Direct3D11Shader();
        
        /* === Shader compilation === */
        
        bool compile(
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "",
            const c8** CompilerOptions = 0, u32 Flags = 0
        );
        
        /* === Set the constant buffer === */
        
        bool setConstantBuffer(const io::stringc &Name, const void* Buffer);
        bool setConstantBuffer(u32 Number, const void* Buffer);
        
        /* === Constant table === */
        
        u32 getConstantCount() const;
        std::vector<io::stringc> getConstantList() const;
        
    private:
        
        friend class Direct3D11ShaderClass;
        friend class Direct3D11RenderSystem;
        
        /* === Functions === */
        
        bool compileHLSL(const c8* ProgramBuffer, const c8* EntryPoint, const c8* TargetName, u32 Flags);
        
        void createProgramString(const std::list<io::stringc> &ShaderBuffer, c8* &ProgramBuffer);
        
        bool createConstantBuffers();
        
        UINT getCompilerFlags(u32 Flags) const;
        
        /* === Inline functions === */
        
        inline u32 getVersionIndex(const EShaderVersions Min, const EShaderVersions Max) const
        {
            return math::MinMax(Version_, Min, Max) - Min;
        }
        
        /* === Members === */
        
        ID3D11Device* D3DDevice_;
        ID3D11DeviceContext* D3DDeviceContext_;
        
        union
        {
            ID3D11VertexShader*     VSObj_;
            ID3D11PixelShader*      PSObj_;
            ID3D11GeometryShader*   GSObj_;
            ID3D11HullShader*       HSObj_;
            ID3D11DomainShader*     DSObj_;
            ID3D11ComputeShader*    CSObj_;
        };

        std::vector<ID3D11Buffer*> HWConstantBuffers_;
        
        ID3D11InputLayout* InputVertexLayout_;
        ID3D11ShaderReflection* ShaderReflection_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
