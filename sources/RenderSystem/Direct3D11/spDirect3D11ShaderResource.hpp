/*
 * Direct3D11 shader resource header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D11_SHADER_RESOURCE_H__
#define __SP_DIRECT3D11_SHADER_RESOURCE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/spShaderResource.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11HardwareBuffer.hpp"


namespace sp
{
namespace video
{


//! Direct3D 11 shader resource (Equivalent to the OpenGL Shader Storage Buffer Object (SSBO)).
class Direct3D11ShaderResource : public ShaderResource, public D3D11HardwareBuffer
{
    
    public:
        
        Direct3D11ShaderResource();
        ~Direct3D11ShaderResource();
        
        /* === Functions === */
        
        bool setupBufferRaw(
            const EShaderResourceTypes Type, u8 AccessFlags, u32 ElementCount, u32 Stride,
            const ERendererDataTypes DataType, u32 DataSize, const void* Buffer = 0
        );
        
        bool writeBuffer(const void* Buffer, u32 Size = 0);
        bool readBuffer(void* Buffer, u32 Size = 0);

        bool copyBuffer(const ShaderResource* SourceBuffer);

        u32 getSize() const;

    private:

        friend class Direct3D11RenderSystem;

        /* === Members === */

        ID3D11ShaderResourceView* ResourceView_;
        ID3D11UnorderedAccessView* AccessView_;

        ID3D11Buffer* CPUAccessBuffer_;

};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
