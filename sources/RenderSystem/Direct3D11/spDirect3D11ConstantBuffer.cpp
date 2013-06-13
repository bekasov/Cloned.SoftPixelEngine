/*
 * Direct3D11 shader constant buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11ConstantBuffer.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11ShaderClass.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Direct3D11ConstantBuffer::Direct3D11ConstantBuffer(
    Direct3D11ShaderClass* Owner, const D3D11_SHADER_BUFFER_DESC &ShaderBufferDesc, u32 Index) :
    ConstantBuffer      (Owner, ShaderBufferDesc.Name, Index),
    D3D11HardwareBuffer (                                   )
{
    Size_ = ShaderBufferDesc.Size;
    
    createBuffer(Size_, 0, HWBUFFER_STATIC, D3D11_BIND_CONSTANT_BUFFER, 0, 0, "constant");
}
Direct3D11ConstantBuffer::~Direct3D11ConstantBuffer()
{
}

bool Direct3D11ConstantBuffer::updateBuffer(const void* Buffer, u32 Size)
{
    setupBuffer(Buffer);
    return true;
}

bool Direct3D11ConstantBuffer::valid() const
{
    return getBufferRef() != 0;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
