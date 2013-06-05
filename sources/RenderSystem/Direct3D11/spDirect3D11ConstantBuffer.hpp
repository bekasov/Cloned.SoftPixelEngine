/*
 * Direct3D11 shader constant buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D11_SHADER_CONSTANT_BUFFER_H__
#define __SP_DIRECT3D11_SHADER_CONSTANT_BUFFER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/spConstantBuffer.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11HardwareBuffer.hpp"

#include <D3D11.h>
#include <D3D11Shader.h>


namespace sp
{
namespace video
{


class Direct3D11ShaderClass;

class /*SP_EXPORT */Direct3D11ConstantBuffer : public ConstantBuffer, public D3D11HardwareBuffer
{
    
    public:
        
        Direct3D11ConstantBuffer(
            Direct3D11ShaderClass* Owner, const D3D11_SHADER_BUFFER_DESC &ShaderBufferDesc, u32 Index
        );
        ~Direct3D11ConstantBuffer();
        
        /* === Functions === */
        
        bool updateBuffer(const void* Buffer, u32 Size = 0);
        
        bool valid() const;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
