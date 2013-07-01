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


Direct3D11ShaderResource::Direct3D11ShaderResource() :
    ShaderResource      (   ),
    D3D11HardwareBuffer (   ),
    ResourceView_       (0  ),
    AccessView_         (0  ),
    CPUAccessBuffer_    (0  )
{
}
Direct3D11ShaderResource::~Direct3D11ShaderResource()
{
    Direct3D11RenderSystem::releaseObject(ResourceView_);
    Direct3D11RenderSystem::releaseObject(AccessView_);
}

bool Direct3D11ShaderResource::setupBufferRaw(
    const EShaderResourceTypes Type, u32 ElementCount, u32 Stride,
    const ERendererDataTypes DataType, u32 DataSize, const void* Buffer)
{
    if (!Stride || !ElementCount)
    {
        io::Log::error("Stride and element-count must not be zero for shader resource");
        return false;
    }

    /* Store new settings */
    const u32 Size = ElementCount * Stride;

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
    else if (isByteAddr())
        MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

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
        ResourceView_ = createShaderResourceView(getBufferRef(), ElementCount, DataType, DataSize);
        return ResourceView_ != 0;
    }
    else
    {
        /* Extract flags from shader resource type */
        s32 Flags = 0;

        if (Type == SHADERRESOURCE_COUNTER_RW_STRUCT_BUFFER)
            Flags |= SHADERBUFFERFLAG_COUNTER;
        else if (Type == SHADERRESOURCE_APPEND_STRUCT_BUFFER)
            Flags |= SHADERBUFFERFLAG_APPEND;

        /* Create unordered access view */
        AccessView_ = createUnorderedAccessView(getBufferRef(), ElementCount, DataType, DataSize, Flags);
        
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

bool Direct3D11ShaderResource::copyBuffer(const ShaderResource* SourceBuffer)
{
    /* Get source buffer and check if size is compatible */
    const Direct3D11ShaderResource* D3DSrcBuffer = static_cast<const Direct3D11ShaderResource*>(SourceBuffer);

    if (!D3DSrcBuffer)
        return false;

    if (getBufferSize() != D3DSrcBuffer->getBufferSize())
    {
        io::Log::error("Incompatible size of shader resources to be copied");
        return false;
    }

    /* Copy buffers */
    ID3D11DeviceContext* D3DDeviceContext = static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->D3DDeviceContext_;

    D3DDeviceContext->CopyResource(getBufferRef(), D3DSrcBuffer->getBufferRef());

    return true;
}

u32 Direct3D11ShaderResource::getSize() const
{
    return getBufferSize();
}


/*
 * ======= Private: =======
 */

ID3D11ShaderResourceView* Direct3D11ShaderResource::createShaderResourceView(
    ID3D11Buffer* HWBuffer, u32 ElementCount, const ERendererDataTypes DataType, u32 DataSize)
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
    ResourceViewDesc.BufferEx.NumElements   = ElementCount;
    
    if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        ResourceViewDesc.Format         = DXGI_FORMAT_R32_TYPELESS;
        ResourceViewDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    }
    else if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
        ResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    else
    {
        ResourceViewDesc.Format = Direct3D11RenderSystem::getDxFormat(DataType, DataSize);

        if (ResourceViewDesc.Format == DXGI_FORMAT_UNKNOWN)
        {
            io::Log::error("Unsupported buffer for shader resource view");
            return 0;
        }
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

ID3D11UnorderedAccessView* Direct3D11ShaderResource::createUnorderedAccessView(
    ID3D11Buffer* HWBuffer, u32 ElementCount, const ERendererDataTypes DataType, u32 DataSize, s32 Flags)
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
    AccessViewDesc.Buffer.NumElements   = ElementCount;
    
    if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        AccessViewDesc.Format       = DXGI_FORMAT_R32_TYPELESS;
        AccessViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
    }
    else if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        AccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;

        if (Flags & SHADERBUFFERFLAG_COUNTER)
            AccessViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
        else if (Flags & SHADERBUFFERFLAG_APPEND)
            AccessViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    }
    else
    {
        AccessViewDesc.Format = Direct3D11RenderSystem::getDxFormat(DataType, DataSize);
        
        if (AccessViewDesc.Format == DXGI_FORMAT_UNKNOWN)
        {
            io::Log::error("Unsupported buffer for unordered access view");
            return 0;
        }
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
