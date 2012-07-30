/*
 * Direct3D9 vertex buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_D3D9_VERTEX_BUFFER_H__
#define __SP_D3D9_VERTEX_BUFFER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "Base/spDimensionUniversalBuffer.hpp"
#include "Base/spMaterialConfigTypes.hpp"

#include <d3d9.h>


namespace sp
{
namespace video
{


#define FVF_POSITION (D3DFVF_XYZ)
#define FVF_VERTEX2D (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
//#define FVF_VERTEX2D (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#define FVF_VERTEX3D                                                \
    ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX8 |   \
      D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) |           \
      D3DFVF_TEXCOORDSIZE3(2) | D3DFVF_TEXCOORDSIZE3(3) |           \
      D3DFVF_TEXCOORDSIZE3(4) | D3DFVF_TEXCOORDSIZE3(5) |           \
      D3DFVF_TEXCOORDSIZE3(6) | D3DFVF_TEXCOORDSIZE3(7) )


class VertexFormat;

//! Direct3D9 vertex buffer class for internal usage.
class D3D9VertexBuffer
{
    
    private:
        
        friend class Direct3D9RenderSystem;
        
        /* Functions */
        
        D3D9VertexBuffer();
        ~D3D9VertexBuffer();
        
        void update(
            IDirect3DDevice9* D3DDevice, const dim::UniversalBuffer &BufferData,
            const VertexFormat* Format, const EMeshBufferUsage Usage
        );
        
        void update(
            IDirect3DDevice9* D3DDevice, const dim::UniversalBuffer &BufferData, u32 Index
        );
        
        /* Members */
        
        s32 FormatFlags_;
        
        u32 VertexCount_;
        u32 BufferSize_;
        
        IDirect3DVertexBuffer9* HWBuffer_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
