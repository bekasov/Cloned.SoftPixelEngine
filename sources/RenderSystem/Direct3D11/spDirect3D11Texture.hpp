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


#if defined(SP_PLATFORM_WINDOWS)
#   include <d3d11.h>
#   include <d3dx11.h>
#endif


namespace sp
{
namespace video
{


class Direct3D11RenderSystem;

class SP_EXPORT Direct3D11Texture : public Texture
{
    
    public:
        
        Direct3D11Texture();
        Direct3D11Texture(
            ID3D11Texture1D* d3dTexture1D, ID3D11Texture2D* d3dTexture2D, ID3D11Texture3D* d3dTexture3D,
            const STextureCreationFlags &CreationFlags
        );
        ~Direct3D11Texture();
        
        bool valid() const;
        
        /* Extra option functions */
        
        void setColorIntensity(f32 Red, f32 Green, f32 Blue);
        
        /* Filter, MipMap filter, wrap modes */
        
        void setFilter(const ETextureFilters Filter);
        void setFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter);
        void setMagFilter(const ETextureFilters Filter);
        void setMinFilter(const ETextureFilters Filter);
        
        void setMipMapFilter(const ETextureMipMapFilters MipMapFilter);
        
        void setWrapMode(const ETextureWrapModes Wrap);
        void setWrapMode(
            const ETextureWrapModes WrapU, const ETextureWrapModes WrapV, const ETextureWrapModes WrapW = TEXWRAP_REPEAT
        );
        
        /* Binding & unbinding */
        
        void bind(s32 Level = 0) const;
        void unbind(s32 Level = 0) const;
        
        /* Functions for updating image buffer */
        
        void shareImageBuffer();
        void updateImageBuffer();
        
    private:
        
        friend class Direct3D11RenderSystem;
        
        /* Private functions */
        
        void clear();
        
        bool recreateHWTexture();
        
        void updateImageTexture();
        
        void updateResource();
        bool updateSamplerState();
        
        void updateMultiRenderTargets();
        
        /* Members */
        
        ID3D11Device* Device_;
        ID3D11DeviceContext* DeviceContext_;
        
        ID3D11Resource* TexResource_;
        ID3D11Texture1D* RendererTexture1D_;
        ID3D11Texture2D* RendererTexture2D_;
        ID3D11Texture3D* RendererTexture3D_;
        
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
