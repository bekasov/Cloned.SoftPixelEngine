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

s32 D3D11PixelFormatDataSize[] = {
    #if 1
    /* 1 component */
    1, // Index color
    4, // Stencil
    1, // Intensity
    1, // Red
    1, // Green
    1, // Blue
    1, // Alpha
    1, // Gray
    
    /* 2 components */
    2, // Gray alpha
    
    /* 3 components */
    4, // RGB
    4, // BGR
    4, // Depth
    
    /* 4 components */
    4, // RGBA
    4  // BGRA
    #else
    1, 1, 2, 4, 4, 4, 4, 1
    #endif
};


/*
 * Direct3D11Texture class
 */

#define mcrD3D11Driver static_cast<Direct3D11RenderSystem*>(__spVideoDriver)

Direct3D11Texture::Direct3D11Texture()
    : Texture(), TexResource_(0), RendererTexture1D_(0), RendererTexture2D_(0), RendererTexture3D_(0),
    ShaderResourceView_(0), RenderTargetView_(0), DepthStencilView_(0), SamplerSate_(0)
{
    /* Default settings */
    DeviceContext_  = mcrD3D11Driver->DeviceContext_;
    Device_         = mcrD3D11Driver->Device_;
    
    memset(RenderTargetViewCubeMap_, 0, sizeof(ID3D11RenderTargetView*)*6);
}
Direct3D11Texture::Direct3D11Texture(
    ID3D11Texture1D* d3dTexture1D, ID3D11Texture2D* d3dTexture2D, ID3D11Texture3D* d3dTexture3D, const STextureCreationFlags &CreationFlags)
    : Texture(CreationFlags), RendererTexture1D_(d3dTexture1D), TexResource_(0), RendererTexture2D_(d3dTexture2D),
    RendererTexture3D_(d3dTexture3D), ShaderResourceView_(0), RenderTargetView_(0), DepthStencilView_(0), SamplerSate_(0)
{
    ID_ = OrigID_ = this;
    
    /* General settings */
    DeviceContext_  = mcrD3D11Driver->DeviceContext_;
    Device_         = mcrD3D11Driver->Device_;
    
    memset(RenderTargetViewCubeMap_, 0, sizeof(ID3D11RenderTargetView*)*6);
    
    AnisotropicSamples_ = CreationFlags.Anisotropy;
    
    if (CreationFlags.ImageBuffer)
        Texture::updateImageBuffer(CreationFlags.ImageBuffer);
}
Direct3D11Texture::~Direct3D11Texture()
{
    clear();
}

bool Direct3D11Texture::valid() const
{
    return ImageBuffer_;
}

void Direct3D11Texture::setColorIntensity(f32 Red, f32 Green, f32 Blue)
{
    
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
        
        if (Level >= mcrD3D11Driver->BindTextureCount_)
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
    /* Update renderer texture format */
    updateFormat();
    
    /* Clear the image data */
    recreateHWTexture();
    
    /* Update renderer image buffer */
    updateImageTexture();
    
    if (MipMaps_)
        DeviceContext_->GenerateMips(ShaderResourceView_);
}


/*
 * ======= Private: =======
 */

void Direct3D11Texture::clear()
{
    for (s32 i = 0; i < 6; ++i)
        Direct3D11RenderSystem::releaseObject(RenderTargetViewCubeMap_[i]);
    
    Direct3D11RenderSystem::releaseObject(RendererTexture1D_);
    Direct3D11RenderSystem::releaseObject(RendererTexture2D_);
    Direct3D11RenderSystem::releaseObject(RendererTexture3D_);
    Direct3D11RenderSystem::releaseObject(ShaderResourceView_);
    Direct3D11RenderSystem::releaseObject(RenderTargetView_);
    Direct3D11RenderSystem::releaseObject(DepthStencilView_);
    Direct3D11RenderSystem::releaseObject(SamplerSate_);
}

void Direct3D11Texture::updateFormat()
{
    /* Update image buffer size */
    if (getFormatSize(Format_) != FormatSize_)
        ImageConverter::convertImageFormat(ImageBuffer_, Size_.Width, Size_.Height, FormatSize_, getFormatSize(Format_));
    
    /* Update only format size */
    updateFormatSize();
}

void Direct3D11Texture::updateFormatSize()
{
    FormatSize_ = D3D11PixelFormatDataSize[Format_];
}
s32 Direct3D11Texture::getFormatSize(const EPixelFormats Format) const
{
    return D3D11PixelFormatDataSize[Format];
}

bool Direct3D11Texture::recreateHWTexture()
{
    /* Delete the old Direct3D11 texture */
    clear();
    
    /* Create the new Direct3D11 texture */
    mcrD3D11Driver->createRendererTexture(
        MipMaps_, DimensionType_, dim::vector3di(Size_.Width, Size_.Height, Depth_), Format_, ImageBuffer_, HWFormat_
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
    if (Device_->CreateShaderResourceView(TexResource_, 0, &ShaderResourceView_))
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
            
            Direct3D11RenderSystem::setupTextureFormats(Format_, HWFormat_, RenderTargetDesc->Format);
            
            RenderTargetDesc->ViewDimension                     = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            RenderTargetDesc->Texture2DArray.FirstArraySlice    = 0;
            RenderTargetDesc->Texture2DArray.ArraySize          = 6;
            RenderTargetDesc->Texture2DArray.MipSlice           = 0;
        }
        
        /* Create render target view */
        if (Device_->CreateRenderTargetView(TexResource_, RenderTargetDesc, &RenderTargetView_))
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
                
                if (Device_->CreateRenderTargetView(TexResource_, RenderTargetDesc, &RenderTargetViewCubeMap_[i]))
                    io::Log::error("Could not create render target view for cube-map face #" + io::stringc(i));
            }
            
            delete RenderTargetDesc;
        }
        
        #if 0
        if (Format_ == PIXELFORMAT_DEPTH)
        {
            /* Create depth stencil view */
            if (Device_->CreateDepthStencilView(TexResource_, 0, &DepthStencilView_))
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
    if (HWFormat_ == HWTEXFORMAT_UBYTE8)
    {
        DeviceContext_->UpdateSubresource(
            TexResource_, 0, 0, ImageBuffer_, Size_.Width*FormatSize_, Size_.Width*Size_.Height*FormatSize_
        );
    }
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
    if (Device_->CreateSamplerState(&SamplerDesc, &SamplerSate_))
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
    
    for (s32 i = 0; i < MultiRenderTargetList_.size(); ++i)
        MRTRenderTargetViewList_[i + 1] = static_cast<Direct3D11Texture*>(MultiRenderTargetList_[i])->RenderTargetView_;
}

#undef mcrD3D11Driver


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
