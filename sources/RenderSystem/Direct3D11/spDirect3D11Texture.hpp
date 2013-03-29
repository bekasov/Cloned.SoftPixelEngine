/*
 * Direct3D11 texture header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_DIRECT3D11TEXTURE_H__
#define __SP_RENDERSYSTEM_DIRECT3D11TEXTURE_H__


#include "Base/spStandard.hpp"
#include "RenderSystem/spTextureBase.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include <d3d11.h>
#include <d3dx11.h>


namespace sp
{
namespace video
{


class Direct3D11RenderSystem;

class SP_EXPORT Direct3D11Texture : public Texture
{
    
    public:
        
        Direct3D11Texture(
            ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DDeviceContext,
            const STextureCreationFlags &CreationFlags
        );
        ~Direct3D11Texture();
        
        /* === Functions === */
        
        bool valid() const;
        
        void setHardwareFormat(const EHWTextureFormats HardwareFormat);
        
        void setFilter(const ETextureFilters Filter);
        void setFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter);
        void setMagFilter(const ETextureFilters Filter);
        void setMinFilter(const ETextureFilters Filter);
        
        void setMipMapFilter(const ETextureMipMapFilters MipMapFilter);
        
        void setWrapMode(const ETextureWrapModes Wrap);
        void setWrapMode(
            const ETextureWrapModes WrapU, const ETextureWrapModes WrapV, const ETextureWrapModes WrapW = TEXWRAP_REPEAT
        );
        
        void bind(s32 Level = 0) const;
        void unbind(s32 Level = 0) const;
        
        bool shareImageBuffer();
        bool updateImageBuffer();
        
    private:
        
        friend class Direct3D11RenderSystem;
        
        /* === Functions === */
        
        void releaseResources();
        
        void setupTextureFormats(DXGI_FORMAT &DxFormat);
        bool createHWTexture();
        
        void updateTextureImage();
        bool updateSamplerState();
        
        bool updateRenderTarget();
        void updateMultiRenderTargets();
        
        /* === Members === */
        
        ID3D11Device* D3DDevice_;
        ID3D11DeviceContext* D3DDeviceContext_;
        
        ID3D11Resource* D3DResource_;
        ID3D11Texture1D* HWTexture1D_;
        ID3D11Texture2D* HWTexture2D_;
        ID3D11Texture3D* HWTexture3D_;
        ID3D11Texture2D* DepthTexture_;
        
        ID3D11ShaderResourceView* ShaderResourceView_;
        ID3D11RenderTargetView* RenderTargetView_;
        ID3D11RenderTargetView* RenderTargetViewCubeMap_[6];
        ID3D11DepthStencilView* DepthStencilView_;
        ID3D11SamplerState* SamplerSate_;
        
        std::vector<ID3D11RenderTargetView*> MRTRenderTargetViewList_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
