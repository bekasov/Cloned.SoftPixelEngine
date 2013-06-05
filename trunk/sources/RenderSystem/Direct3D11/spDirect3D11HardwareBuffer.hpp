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
    
    public:

        D3D11HardwareBuffer();
        virtual ~D3D11HardwareBuffer();
        
        /* === Inline functions === */

        inline ID3D11Buffer* getBufferRef() const
        {
            return HWBuffer_;
        }
        inline ID3D11Buffer*& getBufferRef()
        {
            return HWBuffer_;
        }

        inline u32 getBufferSize() const
        {
            return BufferSize_;
        }

    protected:
        
        friend class Direct3D11RenderSystem;
        #ifdef SP_COMPILE_WITH_OPENCL
        friend class OpenCLBuffer;
        #endif
        
        /* === Functions === */
        
        bool createBuffer(
            u32 Size, u32 Stride, const EHWBufferUsage Usage, u32 BindFlags,
            u32 MiscFlags = 0, const void* Buffer = 0, const io::stringc &DescName = "hardware"
        );
        void deleteBuffer();
        
        void setupBuffer(const void* Buffer);
        bool setupBuffer(
            u32 Size, u32 Stride, const EHWBufferUsage Usage, u32 BindFlags,
            u32 MiscFlags = 0, const void* Buffer = 0, const io::stringc &DescName = "hardware"
        );
        void setupBufferSub(const void* Buffer, u32 Size, u32 Stride, u32 Offset = 0);

    private:

        /* === Members === */
        
        ID3D11Buffer* HWBuffer_;
        
        u32 BufferSize_;

};


typedef D3D11HardwareBuffer D3D11VertexBuffer;

class D3D11IndexBuffer : public D3D11HardwareBuffer
{

    public:

        D3D11IndexBuffer() :
            D3D11HardwareBuffer (                       ),
            Format_             (DXGI_FORMAT_R32_UINT   )
        {
        }
        ~D3D11IndexBuffer()
        {
        }

        inline void setFormat(const DXGI_FORMAT Format)
        {
            Format_ = Format;
        }
        inline DXGI_FORMAT getFormat() const
        {
            return Format_;
        }

    private:

        DXGI_FORMAT Format_;

};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
