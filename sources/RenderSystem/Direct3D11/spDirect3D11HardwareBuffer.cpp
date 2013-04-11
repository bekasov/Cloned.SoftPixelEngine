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
namespace video
{


D3D11HardwareBuffer::D3D11HardwareBuffer() :
    FormatFlags_    (DXGI_FORMAT_R32_UINT   ),
    ElementCount_   (0                      ),
    BufferSize_     (0                      ),
    HWBuffer_       (0                      )
{
    ZeroMemory(&BufferDesc_, sizeof(D3D11_BUFFER_DESC));
}
D3D11HardwareBuffer::~D3D11HardwareBuffer()
{
    if (HWBuffer_)
        HWBuffer_->Release();
}

bool D3D11HardwareBuffer::update(
    ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DDeviceContext,
    const dim::UniversalBuffer &BufferData, const ERendererDataTypes FormatType,
    const EHWBufferUsage Usage, D3D11_BIND_FLAG BindFlag, const io::stringc &Name)
{
    if (!D3DDevice || !D3DDeviceContext)
        return false;
    
    /* Temporary variables */
    const u32 ElementCount  = BufferData.getCount();
    const u32 BufferSize    = BufferData.getSize();
    
    /* Setup format flags */
    FormatFlags_ = (FormatType == DATATYPE_UNSIGNED_INT ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT);
    
    /* Get the hardware mesh buffer */
    D3D11_SUBRESOURCE_DATA ResourceData;
    ZeroMemory(&ResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    
    if (!HWBuffer_ || ElementCount != ElementCount_ || BufferSize != BufferSize_)
    {
        /* Release old hardware vertex buffer */
        Direct3D11RenderSystem::releaseObject(HWBuffer_);
        
        ElementCount_   = ElementCount;
        BufferSize_     = BufferSize;
        
        if (!ElementCount_)
            return true;
        
        /* Setup buffer description */
        BufferDesc_.Usage               = D3D11_USAGE_DEFAULT;
        BufferDesc_.ByteWidth           = BufferSize;
        BufferDesc_.BindFlags           = BindFlag;
        BufferDesc_.CPUAccessFlags      = 0;
        BufferDesc_.StructureByteStride = BufferData.getStride();
        
        /* Create hardware vertex buffer */
        ResourceData.pSysMem = BufferData.getArray();
        
        HRESULT Result = D3DDevice->CreateBuffer(
            &BufferDesc_, &ResourceData, &HWBuffer_
        );
        
        if (Result || !HWBuffer_)
        {
            io::Log::error("Could not create hardware " + Name + " buffer");
            return false;
        }
    }
    else if (ElementCount)
    {
        /* Update hardware vertex buffer */
        D3DDeviceContext->UpdateSubresource(
            HWBuffer_, 0, 0, BufferData.getArray(), 0, 0
        );
    }
    
    return true;
}

bool D3D11HardwareBuffer::update(
    ID3D11DeviceContext* D3DDeviceContext, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (!D3DDeviceContext || !HWBuffer_)
        return false;
    
    /* Setup destination address */
    D3D11_BOX DestAddr;
    ZeroMemory(&DestAddr, sizeof(D3D11_BOX));
    
    const u32 BufferStride = BufferData.getStride();
    
    DestAddr.left = Index * BufferStride; // ???
    
    /* Update hardware vertex buffer */
    D3DDeviceContext->UpdateSubresource(
        HWBuffer_, 0, &DestAddr, BufferData.getArray(Index, 0), BufferStride, 0
    );
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
