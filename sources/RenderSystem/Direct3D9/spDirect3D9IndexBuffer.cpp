/*
 * Direct3D9 index buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D9/spDirect3D9IndexBuffer.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "Base/spIndexFormat.hpp"


namespace sp
{
namespace video
{


D3D9IndexBuffer::D3D9IndexBuffer() :
    FormatFlags_(D3DFMT_INDEX16 ),
    IndexCount_ (0              ),
    BufferSize_ (0              ),
    HWBuffer_   (0              )
{
}
D3D9IndexBuffer::~D3D9IndexBuffer()
{
    if (HWBuffer_)
        HWBuffer_->Release();
}

void D3D9IndexBuffer::update(
    IDirect3DDevice9* D3DDevice, const dim::UniversalBuffer &BufferData,
    const IndexFormat* Format, const EHWBufferUsage Usage)
{
    if (!D3DDevice || !Format)
        return;
    
    /* Setup format flags */
    D3DFORMAT FormatFlags = D3DFMT_INDEX16;
    
    if (Format->getDataType() == DATATYPE_UNSIGNED_INT)
        FormatFlags = D3DFMT_INDEX32;
    
    /* Temporary variables */
    const u32 IndexCount    = BufferData.getCount();
    const u32 BufferSize    = BufferData.getSize();
    
    if (!HWBuffer_ || IndexCount != IndexCount_ || BufferSize != BufferSize_ || FormatFlags != FormatFlags_)
    {
        /* Release old hardware index buffer */
        if (HWBuffer_)
            HWBuffer_->Release();
        
        IndexCount_     = IndexCount;
        FormatFlags_    = FormatFlags;
        BufferSize_     = BufferSize;
        
        /* Create hardware index buffer */
        HRESULT Result = D3DDevice->CreateIndexBuffer(
            BufferSize,
            D3DUSAGE_WRITEONLY | (Usage == HWBUFFER_DYNAMIC ? D3DUSAGE_DYNAMIC : 0),
            FormatFlags_,
            D3DPOOL_DEFAULT,
            &HWBuffer_,
            0
        );
        
        if (Result != D3D_OK || !HWBuffer_)
        {
            io::Log::error("Could not create hardware index buffer");
            return;
        }
    }
    
    if (IndexCount)
    {
        /* Update hardware index buffer */
        void* LockBuffer = 0;
        
        if (HWBuffer_->Lock(0, 0, &LockBuffer, 0) == D3D_OK)
        {
            memcpy(LockBuffer, BufferData.getArray(), BufferSize);
            HWBuffer_->Unlock();
        }
        else
            io::Log::error("Could not update hardware index buffer");
    }
}

void D3D9IndexBuffer::update(
    IDirect3DDevice9* D3DDevice, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (!D3DDevice || !BufferData.getSize())
        return;
    
    /* Temporary variables */
    void* LockBuffer = 0;
    const u32 BufferStride = BufferData.getStride();
    
    /* Update hardware index buffer element */
    if (HWBuffer_->Lock(Index * BufferStride, BufferStride, &LockBuffer, 0) == D3D_OK)
    {
        memcpy(LockBuffer, BufferData.getArray(Index, 0), BufferStride);
        HWBuffer_->Unlock();
    }
    else
        io::Log::error("Could not updater hardware index buffer element");
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
