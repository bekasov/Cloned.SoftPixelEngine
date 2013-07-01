/*
 * Direct3D11 shader class file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11ShaderClass.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


Direct3D11ShaderClass::Direct3D11ShaderClass(const VertexFormat* VertexInputLayout) :
    ShaderClass             (                   ),
    D3DDeviceContext_       (0                  ),
    VertexShaderObject_     (0                  ),
    PixelShaderObject_      (0                  ),
    GeometryShaderObject_   (0                  ),
    HullShaderObject_       (0                  ),
    DomainShaderObject_     (0                  ),
    ComputeShaderObject_    (0                  ),
    VertexConstantBuffers_  (0                  ),
    PixelConstantBuffers_   (0                  ),
    GeometryConstantBuffers_(0                  ),
    HullConstantBuffers_    (0                  ),
    DomainConstantBuffers_  (0                  ),
    ComputeConstantBuffers_ (0                  ),
    InputVertexLayout_      (0                  ),
    VertexFormat_           (VertexInputLayout  )
{
    D3DDeviceContext_  = static_cast<video::Direct3D11RenderSystem*>(__spVideoDriver)->D3DDeviceContext_;
    
    if (VertexInputLayout)
        VertexFormat_ = VertexInputLayout;
    else
        VertexFormat_ = __spVideoDriver->getVertexFormatDefault();
}
Direct3D11ShaderClass::~Direct3D11ShaderClass()
{
}

void Direct3D11ShaderClass::bind(const scene::MaterialNode* Object)
{
    static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->CurShaderClass_ = this;
    
    /* Update shader callbacks */
    if (ObjectCallback_)
        ObjectCallback_(this, Object);
    __spVideoDriver->setSurfaceCallback(SurfaceCallback_);
    
    /* Bind shaders and constant buffers */
    if (VertexShaderObject_)
    {
        D3DDeviceContext_->IASetInputLayout(InputVertexLayout_);
        
        D3DDeviceContext_->VSSetShader(VertexShaderObject_, 0, 0);
        if (!VertexConstantBuffers_->empty())
            D3DDeviceContext_->VSSetConstantBuffers(0, VertexConstantBuffers_->size(), &(*VertexConstantBuffers_)[0]);
    }
    if (PixelShaderObject_)
    {
        D3DDeviceContext_->PSSetShader(PixelShaderObject_, 0, 0);
        if (!PixelConstantBuffers_->empty())
            D3DDeviceContext_->PSSetConstantBuffers(0, PixelConstantBuffers_->size(), &(*PixelConstantBuffers_)[0]);
    }
    if (GeometryShaderObject_)
    {
        D3DDeviceContext_->GSSetShader(GeometryShaderObject_, 0, 0);
        if (!GeometryConstantBuffers_->empty())
            D3DDeviceContext_->GSSetConstantBuffers(0, GeometryConstantBuffers_->size(), &(*GeometryConstantBuffers_)[0]);
    }
    if (HullShaderObject_)
    {
        D3DDeviceContext_->HSSetShader(HullShaderObject_, 0, 0);
        if (!HullConstantBuffers_->empty())
            D3DDeviceContext_->HSSetConstantBuffers(0, HullConstantBuffers_->size(), &(*HullConstantBuffers_)[0]);
    }
    if (DomainShaderObject_)
    {
        D3DDeviceContext_->DSSetShader(DomainShaderObject_, 0, 0);
        if (!DomainConstantBuffers_->empty())
            D3DDeviceContext_->DSSetConstantBuffers(0, DomainConstantBuffers_->size(), &(*DomainConstantBuffers_)[0]);
    }
}

void Direct3D11ShaderClass::unbind()
{
    static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->CurShaderClass_ = 0;
    
    /* Unbind shaders and constant buffers */
    D3DDeviceContext_->IASetInputLayout(0);
    
    D3DDeviceContext_->VSSetShader(0, 0, 0);
    D3DDeviceContext_->VSSetConstantBuffers(0, 0, 0);
    
    D3DDeviceContext_->PSSetShader(0, 0, 0);
    D3DDeviceContext_->PSSetConstantBuffers(0, 0, 0);
    
    D3DDeviceContext_->GSSetShader(0, 0, 0);
    D3DDeviceContext_->GSSetConstantBuffers(0, 0, 0);
    
    D3DDeviceContext_->HSSetShader(0, 0, 0);
    D3DDeviceContext_->HSSetConstantBuffers(0, 0, 0);
    
    D3DDeviceContext_->DSSetShader(0, 0, 0);
    D3DDeviceContext_->DSSetConstantBuffers(0, 0, 0);
}

bool Direct3D11ShaderClass::link()
{
    VertexShaderObject_         = 0;
    PixelShaderObject_          = 0;
    GeometryShaderObject_       = 0;
    HullShaderObject_           = 0;
    DomainShaderObject_         = 0;
    ComputeShaderObject_        = 0;
    
    VertexConstantBuffers_      = 0;
    PixelConstantBuffers_       = 0;
    GeometryConstantBuffers_    = 0;
    HullConstantBuffers_        = 0;
    DomainConstantBuffers_      = 0;
    ComputeConstantBuffers_     = 0;
    
    InputVertexLayout_          = 0;
    
    if (VertexShader_)
    {
        InputVertexLayout_          = static_cast<Direct3D11Shader*>(VertexShader_)->InputVertexLayout_;
        
        VertexShaderObject_         = static_cast<Direct3D11Shader*>(VertexShader_)->VertexShaderObject_;
        VertexConstantBuffers_      = &(static_cast<Direct3D11Shader*>(VertexShader_)->HWConstantBuffers_);
        
        if (!VertexShader_->valid())
            return false;
    }
    else if (!ComputeShader_)
        return false;
    
    if (PixelShader_)
    {
        PixelShaderObject_          = static_cast<Direct3D11Shader*>(PixelShader_)->PixelShaderObject_;
        PixelConstantBuffers_       = &(static_cast<Direct3D11Shader*>(PixelShader_)->HWConstantBuffers_);
        
        if (!PixelShader_->valid())
            return false;
    }
    else
        return false;
    
    if (GeometryShader_)
    {
        GeometryShaderObject_       = static_cast<Direct3D11Shader*>(GeometryShader_)->GeometryShaderObject_;
        GeometryConstantBuffers_    = &(static_cast<Direct3D11Shader*>(GeometryShader_)->HWConstantBuffers_);
        
        if (!GeometryShader_->valid())
            return false;
    }
    if (HullShader_)
    {
        HullShaderObject_           = static_cast<Direct3D11Shader*>(HullShader_)->HullShaderObject_;
        HullConstantBuffers_        = &(static_cast<Direct3D11Shader*>(HullShader_)->HWConstantBuffers_);
        
        if (!HullShader_->valid())
            return false;
    }
    if (DomainShader_)
    {
        DomainShaderObject_         = static_cast<Direct3D11Shader*>(DomainShader_)->DomainShaderObject_;
        DomainConstantBuffers_      = &(static_cast<Direct3D11Shader*>(DomainShader_)->HWConstantBuffers_);
        
        if (!DomainShader_->valid())
            return false;
    }

    if (ComputeShader_)
    {
        ComputeShaderObject_        = static_cast<Direct3D11Shader*>(ComputeShader_)->ComputeShaderObject_;
        ComputeConstantBuffers_     = &(static_cast<Direct3D11Shader*>(ComputeShader_)->HWConstantBuffers_);
        
        if (!ComputeShader_->valid())
            return false;
    }
    
    return true;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
