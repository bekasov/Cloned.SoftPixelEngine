/*
 * Direct3D11 hardware buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_D3D11_HARDWARE_BUFFER_H__
#define __SP_D3D11_HARDWARE_BUFFER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "Base/spDimensionUniversalBuffer.hpp"
#include "Base/spMaterialConfigTypes.hpp"

#include <d3d11.h>


namespace sp
{
namespace video
{


class IndexFormat;

//! Direct3D11 hardware buffer (vertex- or index buffer) class for internal usage.
class D3D11HardwareBuffer
{
    
    private:
        
        friend class Direct3D11RenderSystem;
        #ifdef SP_COMPILE_WITH_OPENCL
        friend class OpenCLBuffer;
        #endif
        
        /* Functions */
        
        D3D11HardwareBuffer();
        ~D3D11HardwareBuffer();
        
        void update(
            ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DDeviceContext,
            const dim::UniversalBuffer &BufferData, const ERendererDataTypes FormatType,
            const EMeshBufferUsage Usage, D3D11_BIND_FLAG BindFlag, const io::stringc &Name
        );
        
        void update(
            ID3D11DeviceContext* D3DDeviceContext, const dim::UniversalBuffer &BufferData, u32 Index
        );
        
        /* Members */
        
        DXGI_FORMAT FormatFlags_;
        
        u32 ElementCount_;
        u32 BufferSize_;
        
        D3D11_BUFFER_DESC BufferDesc_;
        ID3D11Buffer* HWBuffer_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
