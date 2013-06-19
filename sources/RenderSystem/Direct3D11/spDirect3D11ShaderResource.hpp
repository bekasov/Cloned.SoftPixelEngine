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


//! Direct3D 11 texture buffer (Equivalent to the OpenGL Texture Buffer Object (TBO)).
class Direct3D11ShaderResource : public ShaderResource, public D3D11HardwareBuffer
{
    
    public:
        
        Direct3D11ShaderResource(const EShaderResourceTypes Type);
        ~Direct3D11ShaderResource();
        
        /* === Functions === */
        
        bool setupBuffer(
            const EShaderResourceTypes Type, u32 Size, u32 Stride = 0, const void* Buffer = 0
        );
        
        bool writeBuffer(const void* Buffer, u32 Size = 0);
        bool readBuffer(void* Buffer, u32 Size = 0);

        u32 getSize() const;

    private:

        friend class Direct3D11RenderSystem;

        /* === Functions === */

        ID3D11ShaderResourceView* createShaderResourceView(ID3D11Buffer* HWBuffer);
        ID3D11UnorderedAccessView* createUnorderedAccessView(ID3D11Buffer* HWBuffer);

        ID3D11Buffer* createCPUAccessBuffer(ID3D11Buffer* GPUOutputBuffer);

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
