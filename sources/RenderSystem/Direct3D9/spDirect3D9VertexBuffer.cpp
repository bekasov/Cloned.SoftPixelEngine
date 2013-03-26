/*
 * Direct3D9 vertex buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D9/spDirect3D9VertexBuffer.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "Base/spVertexFormat.hpp"


namespace sp
{
namespace video
{


D3D9VertexBuffer::D3D9VertexBuffer() :
    FormatFlags_(0),
    VertexCount_(0),
    BufferSize_ (0),
    HWBuffer_   (0)
{
}
D3D9VertexBuffer::~D3D9VertexBuffer()
{
    if (HWBuffer_)
        HWBuffer_->Release();
}

void D3D9VertexBuffer::update(
    IDirect3DDevice9* D3DDevice, const dim::UniversalBuffer &BufferData,
    const VertexFormat* Format, const EHWBufferUsage Usage)
{
    if (!D3DDevice || !Format)
        return;
    
    /* Setup format flags */
    s32 FormatFlags = 0;
    
    if (Format->getFlags() & VERTEXFORMAT_COORD)
    {
        if (Format->getCoord().Size == 4)
            FormatFlags |= D3DFVF_XYZRHW;
        else
            FormatFlags |= D3DFVF_XYZ;
    }
    if (Format->getFlags() & VERTEXFORMAT_NORMAL)
        FormatFlags |= D3DFVF_NORMAL;
    if (Format->getFlags() & VERTEXFORMAT_COLOR)
        FormatFlags |= D3DFVF_DIFFUSE;
    
    FormatFlags |= (D3DFVF_TEX1 * Format->getTexCoords().size());
    
    if (Format->getFlags() & VERTEXFORMAT_TEXCOORDS)
    {
        for (u32 i = 0; i < Format->getTexCoords().size(); ++i)
        {
            switch (Format->getTexCoords()[i].Size)
            {
                case 1: FormatFlags |= D3DFVF_TEXCOORDSIZE1(i); break;
                case 2: FormatFlags |= D3DFVF_TEXCOORDSIZE2(i); break;
                case 3: FormatFlags |= D3DFVF_TEXCOORDSIZE3(i); break;
                case 4: FormatFlags |= D3DFVF_TEXCOORDSIZE4(i); break;
            }
        }
    }
    
    /* Temporary variables */
    const u32 VertexCount   = BufferData.getCount();
    const u32 BufferSize    = BufferData.getSize();
    
    if (!HWBuffer_ || VertexCount != VertexCount_ || BufferSize != BufferSize_ || FormatFlags != FormatFlags_)
    {
        /* Release old hardware vertex buffer */
        if (HWBuffer_)
            HWBuffer_->Release();
        
        VertexCount_    = VertexCount;
        FormatFlags_    = FormatFlags;
        BufferSize_     = BufferSize;
        
        /* Create hardware vertex buffer */
        HRESULT Result = D3DDevice->CreateVertexBuffer(
            BufferSize,
            D3DUSAGE_WRITEONLY | (Usage == HWBUFFER_DYNAMIC ? D3DUSAGE_DYNAMIC : 0),
            FormatFlags_,
            D3DPOOL_DEFAULT,
            &HWBuffer_,
            0
        );
        
        if (Result != D3D_OK || !HWBuffer_)
        {
            io::Log::error("Could not create hardware vertex buffer");
            return;
        }
    }
    
    if (VertexCount)
    {
        /* Update hardware vertex buffer */
        void* LockBuffer = 0;
        
        if (HWBuffer_->Lock(0, 0, &LockBuffer, 0) == D3D_OK)
        {
            memcpy(LockBuffer, BufferData.getArray(), BufferSize);
            HWBuffer_->Unlock();
        }
        else
            io::Log::error("Could not update hardware vertex buffer");
    }
}

void D3D9VertexBuffer::update(
    IDirect3DDevice9* D3DDevice, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (!D3DDevice || !BufferData.getSize())
        return;
    
    /* Temporary variables */
    void* LockBuffer = 0;
    const u32 BufferStride = BufferData.getStride();
    
    /* Update hardware vertex buffer element */
    if (HWBuffer_->Lock(Index * BufferStride, BufferStride, &LockBuffer, 0) == D3D_OK)
    {
        memcpy(LockBuffer, BufferData.getArray(Index, 0), BufferStride);
        HWBuffer_->Unlock();
    }
    else
        io::Log::error("Could not updater hardware vertex buffer element");
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
