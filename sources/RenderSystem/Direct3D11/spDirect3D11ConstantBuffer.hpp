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

#include <d3d11.h>
#include <d3d11Shader.h>


namespace sp
{
namespace video
{


class Direct3D11Shader;

class SP_EXPORT Direct3D11ConstantBuffer : public ConstantBuffer
{
    
    public:
        
        Direct3D11ConstantBuffer(Direct3D11Shader* Owner, const D3D11_SHADER_BUFFER_DESC &ShaderBufferDesc);
        ~Direct3D11ConstantBuffer();
        
        /* === Functions === */
        
        bool updateBuffer(const void* Buffer, u32 Size = 0);
        
        bool valid() const;
        
    private:
        
        /* === Functions === */
        
        //...
        
        /* === Members === */
        
        ID3D11Buffer* HWBuffer_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
