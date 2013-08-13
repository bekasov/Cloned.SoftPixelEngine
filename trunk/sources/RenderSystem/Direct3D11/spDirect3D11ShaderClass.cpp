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

extern video::RenderSystem* GlbRenderSys;

namespace video
{


Direct3D11ShaderClass::Direct3D11ShaderClass(const VertexFormat* VertexInputLayout) :
    ShaderClass         (                   ),
    D3DDeviceContext_   (0                  ),
    VSObj_              (0                  ),
    PSObj_              (0                  ),
    GSObj_              (0                  ),
    HSObj_              (0                  ),
    DSObj_              (0                  ),
    CSObj_              (0                  ),
    VSConstantBuffers_  (0                  ),
    PSConstantBuffers_  (0                  ),
    GSConstantBuffers_  (0                  ),
    HSConstantBuffers_  (0                  ),
    DSConstantBuffers_  (0                  ),
    CSConstantBuffers_  (0                  ),
    InputVertexLayout_  (0                  ),
    VertexFormat_       (VertexInputLayout  )
{
    D3DDeviceContext_  = static_cast<video::Direct3D11RenderSystem*>(GlbRenderSys)->D3DDeviceContext_;
    
    if (!VertexInputLayout)
        VertexFormat_ = GlbRenderSys->getVertexFormatDefault();
}
Direct3D11ShaderClass::~Direct3D11ShaderClass()
{
}

void Direct3D11ShaderClass::bind(const scene::MaterialNode* Object)
{
    static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->CurShaderClass_ = this;
    
    /* Update shader callbacks */
    if (ObjectCallback_)
        ObjectCallback_(this, Object);
    GlbRenderSys->setSurfaceCallback(SurfaceCallback_);
    
    /* Bind shaders and constant buffers */
    if (VSObj_)
    {
        D3DDeviceContext_->IASetInputLayout(InputVertexLayout_);
        
        D3DDeviceContext_->VSSetShader(VSObj_, 0, 0);
        if (!VSConstantBuffers_->empty())
            D3DDeviceContext_->VSSetConstantBuffers(0, VSConstantBuffers_->size(), &(*VSConstantBuffers_)[0]);
    }
    if (PSObj_)
    {
        D3DDeviceContext_->PSSetShader(PSObj_, 0, 0);
        if (!PSConstantBuffers_->empty())
            D3DDeviceContext_->PSSetConstantBuffers(0, PSConstantBuffers_->size(), &(*PSConstantBuffers_)[0]);
    }
    if (GSObj_)
    {
        D3DDeviceContext_->GSSetShader(GSObj_, 0, 0);
        if (!GSConstantBuffers_->empty())
            D3DDeviceContext_->GSSetConstantBuffers(0, GSConstantBuffers_->size(), &(*GSConstantBuffers_)[0]);
    }
    if (HSObj_)
    {
        D3DDeviceContext_->HSSetShader(HSObj_, 0, 0);
        if (!HSConstantBuffers_->empty())
            D3DDeviceContext_->HSSetConstantBuffers(0, HSConstantBuffers_->size(), &(*HSConstantBuffers_)[0]);
    }
    if (DSObj_)
    {
        D3DDeviceContext_->DSSetShader(DSObj_, 0, 0);
        if (!DSConstantBuffers_->empty())
            D3DDeviceContext_->DSSetConstantBuffers(0, DSConstantBuffers_->size(), &(*DSConstantBuffers_)[0]);
    }
}

void Direct3D11ShaderClass::unbind()
{
    static_cast<Direct3D11RenderSystem*>(GlbRenderSys)->CurShaderClass_ = 0;
    
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
    VSObj_ = 0;
    PSObj_ = 0;
    GSObj_ = 0;
    HSObj_ = 0;
    DSObj_ = 0;
    CSObj_ = 0;
    
    VSConstantBuffers_ = 0;
    PSConstantBuffers_ = 0;
    GSConstantBuffers_ = 0;
    HSConstantBuffers_ = 0;
    DSConstantBuffers_ = 0;
    CSConstantBuffers_ = 0;
    
    InputVertexLayout_ = 0;
    
    if (VertexShader_)
    {
        InputVertexLayout_  = static_cast<Direct3D11Shader*>(VertexShader_)->InputVertexLayout_;
        
        VSObj_              = static_cast<Direct3D11Shader*>(VertexShader_)->VSObj_;
        VSConstantBuffers_  = &(static_cast<Direct3D11Shader*>(VertexShader_)->HWConstantBuffers_);
        
        if (!VertexShader_->valid())
            return false;
    }
    else if (!ComputeShader_)
        return false;
    
    if (PixelShader_)
    {
        PSObj_              = static_cast<Direct3D11Shader*>(PixelShader_)->PSObj_;
        PSConstantBuffers_  = &(static_cast<Direct3D11Shader*>(PixelShader_)->HWConstantBuffers_);
        
        if (!PixelShader_->valid())
            return false;
    }
    else if (!ComputeShader_)
        return false;
    
    if (GeometryShader_)
    {
        GSObj_              = static_cast<Direct3D11Shader*>(GeometryShader_)->GSObj_;
        GSConstantBuffers_  = &(static_cast<Direct3D11Shader*>(GeometryShader_)->HWConstantBuffers_);
        
        if (!GeometryShader_->valid())
            return false;
    }
    if (HullShader_)
    {
        HSObj_              = static_cast<Direct3D11Shader*>(HullShader_)->HSObj_;
        HSConstantBuffers_  = &(static_cast<Direct3D11Shader*>(HullShader_)->HWConstantBuffers_);
        
        if (!HullShader_->valid())
            return false;
    }
    if (DomainShader_)
    {
        DSObj_              = static_cast<Direct3D11Shader*>(DomainShader_)->DSObj_;
        DSConstantBuffers_  = &(static_cast<Direct3D11Shader*>(DomainShader_)->HWConstantBuffers_);
        
        if (!DomainShader_->valid())
            return false;
    }

    if (ComputeShader_)
    {
        if (VertexShader_ || PixelShader_ || GeometryShader_ || HullShader_ || DomainShader_)
        {
            io::Log::error("Compute shader can not be combined with any other shader stage");
            return false;
        }

        CSObj_              = static_cast<Direct3D11Shader*>(ComputeShader_)->CSObj_;
        CSConstantBuffers_  = &(static_cast<Direct3D11Shader*>(ComputeShader_)->HWConstantBuffers_);
        
        if (!ComputeShader_->valid())
            return false;
    }
    else if (!VertexShader_ && !PixelShader_)
        return false;
    
    return true;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
