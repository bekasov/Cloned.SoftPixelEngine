/*
 * Direct3D9 shader table file
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


Direct3D9ShaderTable::Direct3D9ShaderTable()
    : ShaderTable(), pD3D9Device_(0), VertexShaderObject_(0), PixelShaderObject_(0)
{
    pD3D9Device_ = static_cast<video::Direct3D9RenderSystem*>(__spVideoDriver)->getDirect3DDevice();
}
Direct3D9ShaderTable::~Direct3D9ShaderTable()
{
}

void Direct3D9ShaderTable::bind(const scene::MaterialNode* Object)
{
    if (ObjectCallback_)
        ObjectCallback_(this, Object);
    __spVideoDriver->setSurfaceCallback(SurfaceCallback_);
    
    if (VertexShaderObject_)
        pD3D9Device_->SetVertexShader(VertexShaderObject_);
    if (PixelShaderObject_)
        pD3D9Device_->SetPixelShader(PixelShaderObject_);
}

void Direct3D9ShaderTable::unbind()
{
    pD3D9Device_->SetVertexShader(0);
    pD3D9Device_->SetPixelShader(0);
}

bool Direct3D9ShaderTable::link()
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
