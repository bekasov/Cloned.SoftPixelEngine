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


#include <D3D11.h>
#include <D3DX11.h>
#include <boost/shared_ptr.hpp>


namespace sp
{
namespace video
{


union UDx11TexResource
{
    UDx11TexResource() :
        Res(0)
    {
    }
    
    /* Members */
    ID3D11Resource* Res;
    ID3D11Texture1D* Tex1D;
    ID3D11Texture2D* Tex2D;
    ID3D11Texture3D* Tex3D;
};


class Direct3D11RenderSystem;
class D3D11TextureBuffer;

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
        
        void setFilter(const STextureFilter &Filter);
        
        void setMinMagFilter(const ETextureFilters Filter);
        void setMinMagFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter);
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
        
        typedef boost::shared_ptr<D3D11_SHADER_RESOURCE_VIEW_DESC> ResourceViewPtr;
        
        /* === Functions === */
        
        void releaseResources();

        bool createHWTextureResource(
            UDx11TexResource &D3DResource, ResourceViewPtr &ViewDesc,
            bool HasMIPMaps, bool HasCPUAccess, u32 BindFlags, u32 MiscFlags
        );
        
        bool createHWTexture();
        
        bool setupSubResourceData(D3D11_SUBRESOURCE_DATA &SubResourceData);
        void updateTextureImage();
        bool updateSamplerState();
        
        bool createRenderTargetViews();
        bool createDepthTexture();
        void updateMultiRenderTargets();
        void refreshRefMRT();
        
        bool createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC* ViewDescRef);
        bool createUnorderedAccessView();
        
        DXGI_FORMAT getDxTexFormat() const;
        
        /* === Members === */
        
        ID3D11Device* D3DDevice_;
        ID3D11DeviceContext* D3DDeviceContext_;
        
        UDx11TexResource D3DResource_;
        
        ID3D11Texture2D* DepthTexture_;
        
        ID3D11ShaderResourceView* ResourceView_;
        ID3D11UnorderedAccessView* AccessView_;
        
        ID3D11RenderTargetView* RenderTargetView_;
        ID3D11RenderTargetView* RenderTargetViewCubeMap_[6];
        ID3D11DepthStencilView* DepthStencilView_;
        ID3D11SamplerState* SamplerSate_;
        
        std::vector<ID3D11RenderTargetView*> MRTViewList_;
        
        D3D11TextureBuffer* TexBuffer_;     //!< Equivalent to the OpenGL Texture Buffer Object (TBO).
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
