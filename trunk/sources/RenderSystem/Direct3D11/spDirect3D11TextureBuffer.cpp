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


D3D11TextureBuffer::D3D11TextureBuffer(const D3D11_SHADER_BUFFER_DESC &ShaderBufferDesc, u32 Index) :
    D3D11HardwareBuffer()
{
    //Size_ = ShaderBufferDesc.Size;
    
    createBuffer(
        ShaderBufferDesc.Size, 0, HWBUFFER_STATIC,
        D3D11_BIND_SHADER_RESOURCE, 0, 0, "texture"
    );
}
D3D11TextureBuffer::~D3D11TextureBuffer()
{
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
