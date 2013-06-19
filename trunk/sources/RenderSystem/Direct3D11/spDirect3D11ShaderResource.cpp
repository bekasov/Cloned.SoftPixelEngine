/*
 * Direct3D11 shader resource file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11ShaderResource.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Direct3D11ShaderResource::Direct3D11ShaderResource(const EShaderResourceTypes Type) :
    ShaderResource      (Type   ),
    D3D11HardwareBuffer (       ),
    ResourceView_       (0      ),
    AccessView_         (0      ),
    CPUAccessBuffer_    (0      )
{
}
Direct3D11ShaderResource::~Direct3D11ShaderResource()
{
    Direct3D11RenderSystem::releaseObject(ResourceView_);
    Direct3D11RenderSystem::releaseObject(AccessView_);
}

bool Direct3D11ShaderResource::setupBuffer(
    const EShaderResourceTypes Type, u32 Size, u32 Stride, const void* Buffer)
{
    /* Store new settings */
    Type_   = Type;
    Stride_ = Stride;

    /* Setup bind flags */
    u32 BindFlags = D3D11_BIND_SHADER_RESOURCE;

    if (!readOnly())
        BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

    /* Setup misc flags */
    u32 MiscFlags = 0;

    if (isStruct())
        MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    /* Create hardware buffer */
    if (!createBuffer(Size, Stride, HWBUFFER_STATIC, BindFlags, MiscFlags, Buffer, "shader resource"))
        return false;

    /* Delete previous resource views */
    Direct3D11RenderSystem::releaseObject(ResourceView_     );
    Direct3D11RenderSystem::releaseObject(AccessView_       );
    Direct3D11RenderSystem::releaseObject(CPUAccessBuffer_  );

    /* Create shader resource view */
    if (readOnly())
    {
        ResourceView_ = createShaderResourceView(getBufferRef());
        return ResourceView_ != 0;
    }
    else
    {
        /* Create unordered access view */
        AccessView_ = createUnorderedAccessView(getBufferRef());
        
        if (!AccessView_)
            return false;

        /* Create CPU access buffer */
        CPUAccessBuffer_ = createCPUAccessBuffer(getBufferRef());

        if (!CPUAccessBuffer_)
            return false;
    }

    return true;
}

bool Direct3D11ShaderResource::writeBuffer(const void* Buffer, u32 Size)
{
    if (Buffer)
    {
        if (Size > 0)
            D3D11HardwareBuffer::setupBufferSub(Buffer, Size, Stride_);
        else
            D3D11HardwareBuffer::setupBuffer(Buffer);
        return true;
    }
    return false;
}

bool Direct3D11ShaderResource::readBuffer(void* Buffer, u32 Size)
{
    /* Check parameters for validity */
    if (!getBufferRef() || !Buffer)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("Direct3D11ShaderResource::readBuffer");
        #endif
        return false;
    }

    if (!CPUAccessBuffer_)
    {
        io::Log::error("No CPU access buffer created for shader resource");
        return false;
    }
    
    ID3D11DeviceContext* D3DDeviceContext = static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->D3DDeviceContext_;

    if (!Size)
        Size = getBufferSize();
    
    /* Copy resource data from the GPU output buffer to the CPU access buffer */
    D3DDeviceContext->CopyResource(CPUAccessBuffer_, getBufferRef());
    
    /* Map (or rather lock) CPU access buffer */
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    D3DDeviceContext->Map(CPUAccessBuffer_, 0, D3D11_MAP_READ, 0, &MappedResource);
    {
        /* Check buffer validity */
        if (!MappedResource.pData || !MappedResource.DepthPitch)
        {
            io::Log::error("Locking CPU access buffer failed");
            return false;
        }

        if (MappedResource.DepthPitch < Size)
        {
            io::Log::warning(
                "CPU access buffer is too small (trimed from " + io::stringc(Size) +
                " to " + io::stringc(MappedResource.DepthPitch) + " bytes)"
            );
            Size = MappedResource.DepthPitch;
        }
        
        /* Copy the buffer */
        memcpy(Buffer, MappedResource.pData, Size);
    }
    D3DDeviceContext->Unmap(CPUAccessBuffer_, 0);
    
    return true;
}

u32 Direct3D11ShaderResource::getSize() const
{
    return getBufferSize();
}


/*
 * ======= Private: =======
 */

ID3D11ShaderResourceView* Direct3D11ShaderResource::createShaderResourceView(ID3D11Buffer* HWBuffer)
{
    if (!HWBuffer)
        return 0;
    
    /* Get buffer description */
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    HWBuffer->GetDesc(&BufferDesc);
    
    /* Setup resource view description */
    D3D11_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc;
    ZeroMemory(&ResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    
    ResourceViewDesc.ViewDimension          = D3D11_SRV_DIMENSION_BUFFEREX;
    ResourceViewDesc.Buffer.FirstElement    = 0;
    
    if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        ResourceViewDesc.Format                 = DXGI_FORMAT_R32_TYPELESS;
        ResourceViewDesc.BufferEx.Flags         = D3D11_BUFFEREX_SRV_FLAG_RAW;
        ResourceViewDesc.BufferEx.NumElements   = BufferDesc.ByteWidth / 4;
    }
    else if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        ResourceViewDesc.Format                 = DXGI_FORMAT_UNKNOWN;
        ResourceViewDesc.BufferEx.NumElements   = BufferDesc.ByteWidth / BufferDesc.StructureByteStride;
    }
    else
    {
        io::Log::error("Unsupported buffer for shader resource view");
        return 0;
    }
    
    /* Create unordered access view */
    ID3D11ShaderResourceView* ResoruceView = 0;
    
    ID3D11Device* D3DDevice = static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->D3DDevice_;

    if (D3DDevice->CreateShaderResourceView(HWBuffer, &ResourceViewDesc, &ResoruceView))
    {
        io::Log::error("Could not create shader resource view");
        return 0;
    }
    
    return ResoruceView;
}

ID3D11UnorderedAccessView* Direct3D11ShaderResource::createUnorderedAccessView(ID3D11Buffer* HWBuffer)
{
    if (!HWBuffer)
        return 0;
    
    /* Get buffer description */
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    HWBuffer->GetDesc(&BufferDesc);
    
    /* Configure access view description */
    D3D11_UNORDERED_ACCESS_VIEW_DESC AccessViewDesc;
    ZeroMemory(&AccessViewDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
    
    AccessViewDesc.ViewDimension        = D3D11_UAV_DIMENSION_BUFFER;
    AccessViewDesc.Buffer.FirstElement  = 0;
    
    if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        AccessViewDesc.Format               = DXGI_FORMAT_R32_TYPELESS;
        AccessViewDesc.Buffer.Flags         = D3D11_BUFFER_UAV_FLAG_RAW;
        AccessViewDesc.Buffer.NumElements   = BufferDesc.ByteWidth / 4;
    }
    else if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        AccessViewDesc.Format               = DXGI_FORMAT_UNKNOWN;
        AccessViewDesc.Buffer.NumElements   = BufferDesc.ByteWidth / BufferDesc.StructureByteStride;
    }
    else
    {
        io::Log::error("Unsupported buffer for unordered access view");
        return 0;
    }
    
    /* Create unordered access view */
    ID3D11UnorderedAccessView* AccessView = 0;
    
    ID3D11Device* D3DDevice = static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->D3DDevice_;

    if (D3DDevice->CreateUnorderedAccessView(HWBuffer, &AccessViewDesc, &AccessView))
    {
        io::Log::error("Could not create unordered access view");
        return 0;
    }
    
    return AccessView;
}

ID3D11Buffer* Direct3D11ShaderResource::createCPUAccessBuffer(ID3D11Buffer* GPUOutputBuffer)
{
    if (!GPUOutputBuffer)
        return 0;
    
    /* Get buffer description */
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
    GPUOutputBuffer->GetDesc(&BufferDesc);
    
    /* Modify buffer description for CPU access */
    BufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_READ;
    BufferDesc.Usage            = D3D11_USAGE_STAGING;
    BufferDesc.BindFlags        = 0;
    BufferDesc.MiscFlags        = 0;
    
    /* Create CPU access buffer */
    ID3D11Buffer* AccessBuffer  = 0;
    
    ID3D11Device* D3DDevice                 = static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->D3DDevice_;
    ID3D11DeviceContext* D3DDeviceContext   = static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->D3DDeviceContext_;

    if (D3DDevice->CreateBuffer(&BufferDesc, 0, &AccessBuffer))
    {
        io::Log::error("Could not create CPU access buffer");
        return 0;
    }
    
    return AccessBuffer;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
