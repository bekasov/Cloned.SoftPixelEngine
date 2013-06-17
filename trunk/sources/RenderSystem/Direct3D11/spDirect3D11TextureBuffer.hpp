/*
 * Direct3D11 texture buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIRECT3D11_TEXTURE_BUFFER_H__
#define __SP_DIRECT3D11_TEXTURE_BUFFER_H__


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


//! Direct3D 11 texture buffer (Equivalent to the OpenGL Texture Buffer Object (TBO)).
class D3D11TextureBuffer : public D3D11HardwareBuffer
{
    
    public:
        
        D3D11TextureBuffer(u32 Size);
        ~D3D11TextureBuffer();
        
        /* === Functions === */
        
        bool attachBuffer(const void* Buffer);
        void detachBuffer();
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
