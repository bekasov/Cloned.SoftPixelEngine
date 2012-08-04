/*
 * Direct3D9 shader class file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D9/spDirect3D9Shader.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Direct3D9ShaderClass::Direct3D9ShaderClass() :
    ShaderClass         (   ),
    D3DDevice_          (0  ),
    VertexShaderObject_ (0  ),
    PixelShaderObject_  (0  )
{
    D3DDevice_ = static_cast<video::Direct3D9RenderSystem*>(__spVideoDriver)->getDirect3DDevice();
}
Direct3D9ShaderClass::~Direct3D9ShaderClass()
{
}

void Direct3D9ShaderClass::bind(const scene::MaterialNode* Object)
{
    if (ObjectCallback_)
        ObjectCallback_(this, Object);
    __spVideoDriver->setSurfaceCallback(SurfaceCallback_);
    
    if (VertexShaderObject_)
        D3DDevice_->SetVertexShader(VertexShaderObject_);
    if (PixelShaderObject_)
        D3DDevice_->SetPixelShader(PixelShaderObject_);
}

void Direct3D9ShaderClass::unbind()
{
    D3DDevice_->SetVertexShader(0);
    D3DDevice_->SetPixelShader(0);
}

bool Direct3D9ShaderClass::link()
{
    VertexShaderObject_ = 0;
    PixelShaderObject_  = 0;
    
    if (VertexShader_)
        VertexShaderObject_ = static_cast<Direct3D9Shader*>(VertexShader_)->VertexShaderObject_;
    if (PixelShader_)
        PixelShaderObject_ = static_cast<Direct3D9Shader*>(PixelShader_)->PixelShaderObject_;
    
    return VertexShaderObject_ != 0 || PixelShaderObject_ != 0;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
