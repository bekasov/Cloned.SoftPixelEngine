/*
 * Direct3D11 hardware buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11HardwareBuffer.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"
#include "Base/spIndexFormat.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


D3D11HardwareBuffer::D3D11HardwareBuffer() :
    HWBuffer_   (0),
    BufferSize_ (0)
{
}
D3D11HardwareBuffer::~D3D11HardwareBuffer()
{
    deleteBuffer();
}

bool D3D11HardwareBuffer::createBuffer(
    u32 Size, u32 Stride, const EHWBufferUsage Usage, u32 BindFlags,
    u32 MiscFlags, const void* Buffer, const io::stringc &DescName)
{
    ID3D11Device* D3DDevice = static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->D3DDevice_;

    /* Delete previous hardware buffer */
    deleteBuffer();

    /* Setup buffer description */
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    
    BufferDesc.ByteWidth            = Size;
    BufferDesc.Usage                = (Usage == HWBUFFER_DYNAMIC ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT);
    BufferDesc.BindFlags            = BindFlags;
    BufferDesc.CPUAccessFlags       = 0;
    BufferDesc.MiscFlags            = MiscFlags;
    BufferDesc.StructureByteStride  = Stride;
    
    BufferSize_ = Size;
    
    /* Setup resource data */
    D3D11_SUBRESOURCE_DATA ResourceData;
    
    if (Buffer)
    {
        ZeroMemory(&ResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
        ResourceData.pSysMem = Buffer;
    }

    /* Create hardware buffer */
    if (D3DDevice->CreateBuffer(&BufferDesc, Buffer ? &ResourceData : 0, &HWBuffer_) != S_OK)
    {
        io::Log::error("Could not create D3D11 " + DescName + " buffer");
        return false;
    }
    return true;
}

void D3D11HardwareBuffer::deleteBuffer()
{
    Direct3D11RenderSystem::releaseObject(HWBuffer_);
}

void D3D11HardwareBuffer::setupBuffer(const void* Buffer)
{
    if (HWBuffer_ && Buffer)
    {
        /* Update hardware vertex buffer */
        static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->D3DDeviceContext_->UpdateSubresource(
            HWBuffer_, 0, 0, Buffer, 0, 0
        );
    }
}

bool D3D11HardwareBuffer::setupBuffer(
    u32 Size, u32 Stride, const EHWBufferUsage Usage, u32 BindFlags,
    u32 MiscFlags, const void* Buffer, const io::stringc &DescName)
{
    if (HWBuffer_ && BufferSize_ == Size)
    {
        setupBuffer(Buffer);
        return true;
    }
    return createBuffer(Size, Stride, Usage, BindFlags, MiscFlags, Buffer, DescName);
}

void D3D11HardwareBuffer::setupBufferSub(const void* Buffer, u32 Size, u32 Stride, u32 Offset)
{
    if (HWBuffer_ && Buffer && Size > 0)
    {
        /* Setup destination address */
        D3D11_BOX DestAddr;
        ZeroMemory(&DestAddr, sizeof(D3D11_BOX));
        
        DestAddr.left   = Offset;
        DestAddr.right  = Offset + Size;
        
        /* Update hardware vertex buffer */
        static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->D3DDeviceContext_->UpdateSubresource(
            HWBuffer_, 0, &DestAddr, Buffer, Stride, 0
        );
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
