/*
 * Direct3D11 texture buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11TextureBuffer.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


namespace sp
{
namespace video
{


D3D11TextureBuffer::D3D11TextureBuffer(u32 Size) :
    D3D11HardwareBuffer()
{
    createBuffer(
        Size, 0, HWBUFFER_STATIC, D3D11_BIND_SHADER_RESOURCE, 0, 0, "texture"
        //BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE ...
    );
}
D3D11TextureBuffer::~D3D11TextureBuffer()
{
}

bool D3D11TextureBuffer::attachBuffer(const void* Buffer)
{
    setupBuffer(Buffer);
    return true;
}
void D3D11TextureBuffer::detachBuffer()
{
    //todo
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
