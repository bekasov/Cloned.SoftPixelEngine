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

extern video::RenderSystem* GlbRenderSys;

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
    const EShaderResourceTypes Type, u8 AccessFlags, u32 ElementCount, u32 Stride,
    const ERendererDataTypes DataType, u32 DataSize, const void* Buffer)
{
    if (!validateParameters(AccessFlags, ElementCount, Stride))
        return false;

    /* Store new settings */
    const u32 Size = ElementCount * Stride;

    Type_   = Type;
    Stride_ = Stride;

    /* Setup bind flags */
    u32 BindFlags = 0;
    
    if (readAccess())
        BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (writeAccess())
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

    if (readAccess())
    {
        /* Create shader resource view */
        ResourceView_ = D3D11_RENDER_SYS->createShaderResourceView(getBufferRef(), ElementCount, DataType, DataSize);

        if (!ResourceView_)
            return false;
    }

    if (writeAccess())
    {
        /* Extract flags from shader resource type */
        s32 Flags = 0;

        if (Type == SHADERRESOURCE_COUNTER_STRUCT_BUFFER)
            Flags |= SHADERBUFFERFLAG_COUNTER;
        else if (Type == SHADERRESOURCE_APPEND_STRUCT_BUFFER)
            Flags |= SHADERBUFFERFLAG_APPEND;

        /* Create unordered access view */
        AccessView_ = D3D11_RENDER_SYS->createUnorderedAccessView(getBufferRef(), ElementCount, DataType, DataSize, Flags);
        
        if (!AccessView_)
            return false;

        /* Create CPU access buffer */
        CPUAccessBuffer_ = D3D11_RENDER_SYS->createCPUAccessBuffer(getBufferRef());

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
    
    ID3D11DeviceContext* D3DDeviceContext = static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->D3DDeviceContext_;

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
    ID3D11DeviceContext* D3DDeviceContext = static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->D3DDeviceContext_;

    D3DDeviceContext->CopyResource(getBufferRef(), D3DSrcBuffer->getBufferRef());

    return true;
}

u32 Direct3D11ShaderResource::getSize() const
{
    return getBufferSize();
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
