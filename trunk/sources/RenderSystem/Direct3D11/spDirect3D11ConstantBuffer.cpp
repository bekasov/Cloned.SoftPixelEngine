/*
 * Direct3D11 shader constant buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11ConstantBuffer.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11ShaderClass.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Direct3D11ConstantBuffer::Direct3D11ConstantBuffer(
    Direct3D11ShaderClass* Owner, const D3D11_SHADER_BUFFER_DESC &ShaderBufferDesc) :
    ConstantBuffer  (Owner, ShaderBufferDesc.Name   ),
    HWBuffer_       (0                              )
{
    ID3D11Device* D3DDevice = static_cast<video::Direct3D11RenderSystem*>(__spVideoDriver)->D3DDevice_;
    
    /* Create hardware buffer */
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    
    BufferDesc.Usage            = D3D11_USAGE_DEFAULT;
    BufferDesc.ByteWidth        = ShaderBufferDesc.Size;
    BufferDesc.BindFlags        = D3D11_BIND_CONSTANT_BUFFER;
    BufferDesc.CPUAccessFlags   = 0;
    
    if (D3DDevice->CreateBuffer(&BufferDesc, 0, &HWBuffer_))
    {
        io::Log::error("Could not create shader constant buffer \"" + getName() + "\"");
        HWBuffer_ = 0;
    }
}
Direct3D11ConstantBuffer::~Direct3D11ConstantBuffer()
{
    Direct3D11RenderSystem::releaseObject(HWBuffer_);
}

bool Direct3D11ConstantBuffer::updateBuffer(const void* Buffer, u32 Size)
{
    if (HWBuffer_)
    {
        ID3D11DeviceContext* D3DDeviceContext = static_cast<video::Direct3D11RenderSystem*>(__spVideoDriver)->D3DDeviceContext_;
        
        /* Update constant buffer data */
        D3DDeviceContext->UpdateSubresource(HWBuffer_, 0, 0, Buffer, 0, 0);
        
        return true;
    }
    return false;
}

bool Direct3D11ConstantBuffer::valid() const
{
    return HWBuffer_ != 0;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
