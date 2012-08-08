/*
 * Direct3D11 texture file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11Texture.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"
#include "Base/spImageManagement.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


/*
 * Internal members
 */

const D3D11_TEXTURE_ADDRESS_MODE D3D11TextureWrapModes[] =
{
    D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_CLAMP,
};


/*
 * Direct3D11Texture class
 */

#define mcrD3D11Driver static_cast<Direct3D11RenderSystem*>(__spVideoDriver)

Direct3D11Texture::Direct3D11Texture(
    ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DDeviceContext) :
    Texture             (                   ),
    D3DDevice_          (D3DDevice          ),
    D3DDeviceContext_   (D3DDeviceContext   ),
    TexResource_        (0                  ),
    RendererTexture1D_  (0                  ),
    RendererTexture2D_  (0                  ),
    RendererTexture3D_  (0                  ),
    ShaderResourceView_ (0                  ),
    RenderTargetView_   (0                  ),
    DepthStencilView_   (0                  ),
    SamplerSate_        (0                  )
{
    memset(RenderTargetViewCubeMap_, 0, sizeof(ID3D11RenderTargetView*)*6);
}
Direct3D11Texture::Direct3D11Texture(
    ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DDeviceContext,
    ID3D11Texture1D* D3DTexture1D, ID3D11Texture2D* D3DTexture2D, ID3D11Texture3D* D3DTexture3D,
    const STextureCreationFlags &CreationFlags) :
    Texture             (CreationFlags      ),
    D3DDevice_          (D3DDevice          ),
    D3DDeviceContext_   (D3DDeviceContext   ),
    TexResource_        (0                  ),
    RendererTexture1D_  (D3DTexture1D       ),
    RendererTexture2D_  (D3DTexture2D       ),
    RendererTexture3D_  (D3DTexture3D       ),
    ShaderResourceView_ (0                  ),
    RenderTargetView_   (0                  ),
    DepthStencilView_   (0                  ),
    SamplerSate_        (0                  )
{
    ID_ = OrigID_ = this;
    
    memset(RenderTargetViewCubeMap_, 0, sizeof(ID3D11RenderTargetView*)*6);
    
    AnisotropicSamples_ = CreationFlags.Anisotropy;
    
    if (CreationFlags.ImageBuffer)
        updateImageBuffer();
}
Direct3D11Texture::~Direct3D11Texture()
{
    releaseResources();
}

bool Direct3D11Texture::valid() const
{
    return TexResource_ != 0;
}

void Direct3D11Texture::setColorIntensity(f32 Red, f32 Green, f32 Blue)
{
    //todo
}


/* === Filter, MipMapping, Texture coordinate wraps === */

void Direct3D11Texture::setFilter(const ETextureFilters Filter)
{
    if (MagFilter_ != Filter || MinFilter_ != Filter)
    {
        MagFilter_ = MinFilter_ = Filter;
        updateSamplerState();
    }
}
void Direct3D11Texture::setFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter)
{
    if (MagFilter_ != MagFilter || MinFilter_ != MinFilter)
    {
        MagFilter_ = MagFilter;
        MinFilter_ = MinFilter;
        updateSamplerState();
    }
}
void Direct3D11Texture::setMagFilter(const ETextureFilters Filter)
{
    if (MagFilter_ != Filter)
    {
        MagFilter_ = Filter;
        updateSamplerState();
    }
}
void Direct3D11Texture::setMinFilter(const ETextureFilters Filter)
{
    if (MinFilter_ != Filter)
    {
        MinFilter_ = Filter;
        updateSamplerState();
    }
}

void Direct3D11Texture::setMipMapFilter(const ETextureMipMapFilters MipMapFilter)
{
    if (MipMapFilter_ != MipMapFilter)
    {
        MipMapFilter_ = MipMapFilter;
        updateSamplerState();
    }
}

void Direct3D11Texture::setWrapMode(const ETextureWrapModes Wrap)
{
    if (WrapMode_.X != Wrap || WrapMode_.Y != Wrap || WrapMode_.Z != Wrap)
    {
        WrapMode_.X = WrapMode_.Y = WrapMode_.Z = Wrap;
        updateSamplerState();
    }
}
void Direct3D11Texture::setWrapMode(
    const ETextureWrapModes WrapU, const ETextureWrapModes WrapV, const ETextureWrapModes WrapW)
{
    if (WrapMode_.X != WrapU || WrapMode_.Y != WrapV || WrapMode_.Z != WrapW)
    {
        WrapMode_.X = WrapU;
        WrapMode_.Y = WrapV;
        WrapMode_.Z = WrapW;
        updateSamplerState();
    }
}


/* === Binding/ unbinding & image buffer === */

void Direct3D11Texture::bind(s32 Level) const
{
    if (Level < MAX_COUNT_OF_TEXTURES)
    {
        Direct3D11Texture* Tex = static_cast<Direct3D11Texture*>(ID_);
        
        if (Level >= static_cast<s32>(mcrD3D11Driver->BindTextureCount_))
            mcrD3D11Driver->BindTextureCount_ = Level + 1;
        
        mcrD3D11Driver->ShaderResourceViewList_[Level]  = Tex->ShaderResourceView_;
        mcrD3D11Driver->SamplerStateList_[Level]        = Tex->SamplerSate_;
    }
}

void Direct3D11Texture::unbind(s32 Level) const
{
    if (Level < MAX_COUNT_OF_TEXTURES)
    {
        mcrD3D11Driver->ShaderResourceViewList_[Level]  = 0;
        mcrD3D11Driver->SamplerStateList_[Level]        = 0;
    }
}

void Direct3D11Texture::shareImageBuffer()
{
    
}

void Direct3D11Texture::updateImageBuffer()
{
    /* Clear the image data */
    recreateHWTexture();
    
    /* Update renderer image buffer */
    updateImageTexture();
    
    if (MipMaps_)
        D3DDeviceContext_->GenerateMips(ShaderResourceView_);
}


/*
 * ======= Private: =======
 */

void Direct3D11Texture::releaseResources()
{
    for (s32 i = 0; i < 6; ++i)
        Direct3D11RenderSystem::releaseObject(RenderTargetViewCubeMap_[i]);
    
    Direct3D11RenderSystem::releaseObject(RendererTexture1D_    );
    Direct3D11RenderSystem::releaseObject(RendererTexture2D_    );
    Direct3D11RenderSystem::releaseObject(RendererTexture3D_    );
    Direct3D11RenderSystem::releaseObject(ShaderResourceView_   );
    Direct3D11RenderSystem::releaseObject(RenderTargetView_     );
    Direct3D11RenderSystem::releaseObject(DepthStencilView_     );
    Direct3D11RenderSystem::releaseObject(SamplerSate_          );
}

bool Direct3D11Texture::recreateHWTexture()
{
    if (ImageBuffer_->getType() != IMAGEBUFFER_UBYTE)
        return false;
    
    /* Adjust format size */
    ImageBuffer_->adjustFormatD3D();
    
    /* Delete the old Direct3D11 texture */
    releaseResources();
    
    /* Create the new Direct3D11 texture */
    mcrD3D11Driver->createRendererTexture(
        MipMaps_,
        DimensionType_,
        ImageBuffer_->getSizeVector(),
        ImageBuffer_->getFormat(),
        static_cast<const u8*>(ImageBuffer_->getBuffer()),
        HWFormat_
    );
    
    switch (DimensionType_)
    {
        case TEXTURE_1D:
            TexResource_ = RendererTexture1D_ = mcrD3D11Driver->CurTexture1D_; break;
        case TEXTURE_CUBEMAP:
        case TEXTURE_2D:
            TexResource_ = RendererTexture2D_ = mcrD3D11Driver->CurTexture2D_; break;
        case TEXTURE_3D:
            TexResource_ = RendererTexture3D_ = mcrD3D11Driver->CurTexture3D_; break;
    }
    
    /* Update sampler state */
    updateSamplerState();
    
    /* Create shader resource view */
    if (D3DDevice_->CreateShaderResourceView(TexResource_, 0, &ShaderResourceView_))
    {
        io::Log::error("Could not create shader resource view");
        return false;
    }
    
    if (isRenderTarget_)
    {
        D3D11_RENDER_TARGET_VIEW_DESC* RenderTargetDesc = 0;
        
        /* Configure render target description for cube-maps */
        if (DimensionType_ == TEXTURE_CUBEMAP)
        {
            RenderTargetDesc = new D3D11_RENDER_TARGET_VIEW_DESC;
            
            Direct3D11RenderSystem::setupTextureFormats(
                ImageBuffer_->getFormat(), HWFormat_, RenderTargetDesc->Format
            );
            
            RenderTargetDesc->ViewDimension                     = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            RenderTargetDesc->Texture2DArray.FirstArraySlice    = 0;
            RenderTargetDesc->Texture2DArray.ArraySize          = 6;
            RenderTargetDesc->Texture2DArray.MipSlice           = 0;
        }
        
        /* Create render target view */
        if (D3DDevice_->CreateRenderTargetView(TexResource_, RenderTargetDesc, &RenderTargetView_))
        {
            io::Log::error("Could not create render target view");
            return false;
        }
        
        /* Create render target view for cube-maps */
        if (DimensionType_ == TEXTURE_CUBEMAP)
        {
            RenderTargetDesc->Texture2DArray.ArraySize = 1;
            
            for (s32 i = 0; i < 6; ++i)
            {
                RenderTargetDesc->Texture2DArray.FirstArraySlice = i;
                
                if (D3DDevice_->CreateRenderTargetView(TexResource_, RenderTargetDesc, &RenderTargetViewCubeMap_[i]))
                    io::Log::error("Could not create render target view for cube-map face #" + io::stringc(i));
            }
            
            delete RenderTargetDesc;
        }
        
        #if 0
        if (Format_ == PIXELFORMAT_DEPTH)
        {
            /* Create depth stencil view */
            if (D3DDevice_->CreateDepthStencilView(TexResource_, 0, &DepthStencilView_))
            {
                io::Log::error("Could not create depth stencil view");
                return false;
            }
        }
        #endif
    }
    
    return true;
}


void Direct3D11Texture::updateImageTexture()
{
    if (HWFormat_ != HWTEXFORMAT_UBYTE8)
        return;
    
    const dim::size2di Size(ImageBuffer_->getSize());
    const u32 FormatSize = ImageBuffer_->getFormatSize();
    
    D3DDeviceContext_->UpdateSubresource(
        TexResource_, 0, 0, ImageBuffer_->getBuffer(),
        FormatSize*Size.Width,
        FormatSize*Size.Width*Size.Height
    );
}


void Direct3D11Texture::updateResource()
{
    switch (DimensionType_)
    {
        case TEXTURE_1D:
            TexResource_ = RendererTexture1D_; break;
        case TEXTURE_2D:
            TexResource_ = RendererTexture2D_; break;
        case TEXTURE_3D:
            TexResource_ = RendererTexture3D_; break;
    }
}

bool Direct3D11Texture::updateSamplerState()
{
    /* Delete the old sampler state */
    Direct3D11RenderSystem::releaseObject(SamplerSate_);
    
    /* Sampler description */
    D3D11_SAMPLER_DESC SamplerDesc;
    ZeroMemory(&SamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    
    /* Wrap modes (reapeat, mirror, clamp) */
    SamplerDesc.AddressU = D3D11TextureWrapModes[WrapMode_.X];
    SamplerDesc.AddressV = D3D11TextureWrapModes[WrapMode_.Y];
    SamplerDesc.AddressW = D3D11TextureWrapModes[WrapMode_.Z];
    
    /* Magnification, minification and MIP-mapping filter */
    if (MipMapFilter_ != FILTER_ANISOTROPIC)
    {
        if (MagFilter_ == FILTER_SMOOTH)
        {
            if (MinFilter_ == FILTER_SMOOTH)
            {
                if (MipMapFilter_ == FILTER_TRILINEAR)
                    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                else
                    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            }
            else
            {
                if (MipMapFilter_ == FILTER_TRILINEAR)
                    SamplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
                else
                    SamplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }
        }
        else
        {
            if (MinFilter_ == FILTER_SMOOTH)
            {
                if (MipMapFilter_ == FILTER_TRILINEAR)
                    SamplerDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                else
                    SamplerDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            }
            else
            {
                if (MipMapFilter_ == FILTER_TRILINEAR)
                    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                else
                    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            }
        }
    }
    else
        SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    
    /* Anisotropy */
    SamplerDesc.MaxAnisotropy   = (MipMapFilter_ == FILTER_ANISOTROPIC ? AnisotropicSamples_ : 0);
    
    /* Other descriptions */
    SamplerDesc.ComparisonFunc  = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD          = 0;
    SamplerDesc.MaxLOD          = D3D11_FLOAT32_MAX;
    
    /* Create the sampler state */
    if (D3DDevice_->CreateSamplerState(&SamplerDesc, &SamplerSate_))
    {
        io::Log::error("Could not create sampler state");
        return false;
    }
    
    return true;
}

void Direct3D11Texture::updateMultiRenderTargets()
{
    MRTRenderTargetViewList_.resize(MultiRenderTargetList_.size() + 1);
    
    MRTRenderTargetViewList_[0] = RenderTargetView_;
    
    for (u32 i = 0; i < MultiRenderTargetList_.size(); ++i)
        MRTRenderTargetViewList_[i + 1] = static_cast<Direct3D11Texture*>(MultiRenderTargetList_[i])->RenderTargetView_;
}

#undef mcrD3D11Driver


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
