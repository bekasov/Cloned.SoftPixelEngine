/*
 * Direct3D9 shader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D9_SHADER_H__
#define __SP_DIRECT3D9_SHADER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/Direct3D9/spDirect3D9ShaderClass.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <d3d9.h>
#   include <d3dx9shader.h>
#endif


namespace sp
{
namespace video
{


class SP_EXPORT Direct3D9Shader : public Shader
{
    
    public:
        
        Direct3D9Shader(ShaderClass* ShdClass, const EShaderTypes Type, const EShaderVersions Version);
        ~Direct3D9Shader();
        
        /* === Shader compilation === */
        
        bool compile(
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "",
            const c8** CompilerOptions = 0, u32 Flags = 0
        );
        
        /* === Set the constants (by number) === */
        
        bool setConstant(s32 Number, const EConstantTypes Type, const f32 Value);
        bool setConstant(s32 Number, const EConstantTypes Type, const f32* Buffer, s32 Count);
        
        bool setConstant(s32 Number, const EConstantTypes Type, const dim::vector3df &Position);
        bool setConstant(s32 Number, const EConstantTypes Type, const video::color &Color);
        bool setConstant(s32 Number, const EConstantTypes Type, const dim::matrix4f &Matrix);
        
        /* === Set the constants (by name) === */
        
        bool setConstant(const io::stringc &Name, const f32 Value);
        bool setConstant(const io::stringc &Name, const f32* Buffer, s32 Count);
        
        bool setConstant(const io::stringc &Name, const s32 Value);
        bool setConstant(const io::stringc &Name, const s32* Buffer, s32 Count);
        
        bool setConstant(const io::stringc &Name, const dim::vector3df &Position);
        bool setConstant(const io::stringc &Name, const dim::vector4df &Position);
        bool setConstant(const io::stringc &Name, const video::color &Color);
        bool setConstant(const io::stringc &Name, const dim::matrix4f &Matrix);
        
        /* === Set the constants for assembly shaders === */
        
        bool setConstant(const f32* Buffer, s32 StartRegister, s32 ConstAmount);
        
    private:
        
        friend class Direct3D9ShaderClass;
        
        /* === Functions === */
        
        bool compileHLSL(const c8* ProgramBuffer, const c8* EntryPoint, const c8* TargetName, u32 Flags);
        bool compileProgram(const c8* ProgramBuffer);
        
        HRESULT d3dAssembleShader(
            LPCSTR pSrcData, UINT SrcDataLen, CONST D3DXMACRO* pDefines,
            LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs
        );
        
        HRESULT d3dCompileShader(
            LPCSTR pSrcData, UINT SrcDataLen, CONST D3DXMACRO* pDefines,
            LPD3DXINCLUDE pInclude, LPCSTR pFunctionName,
            LPCSTR pProfile, DWORD Flags, LPD3DXBUFFER* ppShader,
            LPD3DXBUFFER* ppErrorMsgs, LPD3DXCONSTANTTABLE* ppConstantTable
        );
        
        bool setupShaderConstants();
        
        /* === Members === */
        
        IDirect3DDevice9* D3DDevice_;
        
        IDirect3DVertexShader9* VertexShaderObject_;
        IDirect3DPixelShader9* PixelShaderObject_;
        
        ID3DXConstantTable* ConstantTable_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
