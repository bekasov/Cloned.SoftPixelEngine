/*
 * Direct3D9 index buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_D3D9_INDEX_BUFFER_H__
#define __SP_D3D9_INDEX_BUFFER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "Base/spDimensionUniversalBuffer.hpp"
#include "Base/spMaterialConfigTypes.hpp"

#include <d3d9.h>


namespace sp
{
namespace video
{


class IndexFormat;

//! Direct3D9 index buffer class for internal usage.
class D3D9IndexBuffer
{
    
    private:
        
        friend class Direct3D9RenderSystem;
        
        /* Functions */
        
        D3D9IndexBuffer();
        ~D3D9IndexBuffer();
        
        void update(
            IDirect3DDevice9* D3DDevice, const dim::UniversalBuffer &BufferData,
            const IndexFormat* Format, const EHWBufferUsage Usage
        );
        
        void update(
            IDirect3DDevice9* D3DDevice, const dim::UniversalBuffer &BufferData, u32 Index
        );
        
        /* Members */
        
        D3DFORMAT FormatFlags_;
        
        u32 IndexCount_;
        u32 BufferSize_;
        
        IDirect3DIndexBuffer9* HWBuffer_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
