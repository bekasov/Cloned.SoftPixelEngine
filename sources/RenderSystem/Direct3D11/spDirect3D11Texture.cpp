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

const DXGI_FORMAT D3D11TexInternalFormatListUByte8[] = {
    DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT
};

const DXGI_FORMAT D3D11TexInternalFormatListFloat16[] = {
    DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT
};

const DXGI_FORMAT D3D11TexInternalFormatListFloat32[] = {
    DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT
};


/*
 * Direct3D11Texture class
 */

#define mcrD3D11Driver static_cast<Direct3D11RenderSystem*>(__spVideoDriver)

Direct3D11Texture::Direct3D11Texture(
    ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DDeviceContext, const STextureCreationFlags &CreationFlags) :
    Texture             (CreationFlags      ),
    D3DDevice_          (D3DDevice          ),
    D3DDeviceContext_   (D3DDeviceContext   ),
    D3DResource_        (0                  ),
    HWTexture1D_        (0                  ),
    HWTexture2D_        (0                  ),
    HWTexture3D_        (0                  ),
    DepthTexture_       (0                  ),
    ShaderResourceView_ (0                  ),
    RenderTargetView_   (0                  ),
    DepthStencilView_   (0                  ),
    SamplerSate_        (0                  )
{
    memset(RenderTargetViewCubeMap_, 0, sizeof(ID3D11RenderTargetView*)*6);
    
    ID_ = OrigID_ = this;
    
    if (CreationFlags.ImageBuffer)
        updateImageBuffer();
    else
        createHWTexture();
}
Direct3D11Texture::~Direct3D11Texture()
{
    releaseResources();
}

bool Direct3D11Texture::valid() const
{
    return D3DResource_ != 0;
}

void Direct3D11Texture::setHardwareFormat(const EHWTextureFormats HardwareFormat)
{
    HWFormat_ = HardwareFormat;
    
    /* Update image buffer data type */
    
    
    
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

bool Direct3D11Texture::shareImageBuffer()
{
    return false; //todo
}

bool Direct3D11Texture::updateImageBuffer()
{
    /* Re-create the hardware texture */
    if (!createHWTexture())
        return false;
    
    /* Update renderer image buffer */
    updateTextureImage();
    
    if (MipMaps_)
        D3DDeviceContext_->GenerateMips(ShaderResourceView_);
    
    return true;
}


/*
 * ======= Private: =======
 */

void Direct3D11Texture::releaseResources()
{
    for (s32 i = 0; i < 6; ++i)
        Direct3D11RenderSystem::releaseObject(RenderTargetViewCubeMap_[i]);
    
    Direct3D11RenderSystem::releaseObject(HWTexture1D_          );
    Direct3D11RenderSystem::releaseObject(HWTexture2D_          );
    Direct3D11RenderSystem::releaseObject(HWTexture3D_          );
    Direct3D11RenderSystem::releaseObject(DepthTexture_         );
    Direct3D11RenderSystem::releaseObject(ShaderResourceView_   );
    Direct3D11RenderSystem::releaseObject(RenderTargetView_     );
    Direct3D11RenderSystem::releaseObject(DepthStencilView_     );
    Direct3D11RenderSystem::releaseObject(SamplerSate_          );
}

void Direct3D11Texture::setupTextureFormats(DXGI_FORMAT &DxFormat)
{
    const EPixelFormats Format = ImageBuffer_->getFormat();
    
    if (Format >= PIXELFORMAT_ALPHA && Format <= PIXELFORMAT_DEPTH)
    {
        switch (HWFormat_)
        {
            case HWTEXFORMAT_UBYTE8:
                DxFormat = D3D11TexInternalFormatListUByte8[Format]; break;
            case HWTEXFORMAT_FLOAT16:
                DxFormat = D3D11TexInternalFormatListFloat16[Format]; break;
            case HWTEXFORMAT_FLOAT32:
                DxFormat = D3D11TexInternalFormatListFloat32[Format]; break;
        }
    }
}

bool Direct3D11Texture::createHWTexture()
{
    /* Delete old Direct3D11 resources */
    releaseResources();
    
    /* Adjust format size */
    ImageBuffer_->adjustFormatD3D();
    
    /* Direct3D11 texture format setup */
    dim::vector3di Size(ImageBuffer_->getSizeVector());
    
    HRESULT Result = 0;
    DXGI_FORMAT DxFormat = DXGI_FORMAT_UNKNOWN;
    
    setupTextureFormats(DxFormat);
    
    //if (Size.Z > 1)
    //    Size.Y /= Size.Z;
    
    /* Create a new Direct3D11 texture */
    switch (DimensionType_)
    {
        case TEXTURE_1D:
        {
            /* Initialize texture description */
            D3D11_TEXTURE1D_DESC TextureDesc;
            
            TextureDesc.Width               = Size.X;
            TextureDesc.MipLevels           = (MipMaps_ ? 0 : 1);
            TextureDesc.ArraySize           = 1;
            TextureDesc.Format              = DxFormat;
            TextureDesc.Usage               = D3D11_USAGE_DEFAULT;
            TextureDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            TextureDesc.CPUAccessFlags      = 0;
            TextureDesc.MiscFlags           = D3D11_RESOURCE_MISC_GENERATE_MIPS;
            
            /* Create the 1 dimensional texture */
            Result = D3DDevice_->CreateTexture1D(&TextureDesc, 0, &HWTexture1D_);
            D3DResource_ = HWTexture1D_;
        }
        break;
        
        case TEXTURE_2D:
        {
            /* Initialize texture description */
            D3D11_TEXTURE2D_DESC TextureDesc;
            
            TextureDesc.Width               = Size.X;
            TextureDesc.Height              = Size.Y;
            TextureDesc.MipLevels           = (MipMaps_ ? 0 : 1);
            TextureDesc.ArraySize           = 1;
            TextureDesc.Format              = DxFormat;
            TextureDesc.Usage               = D3D11_USAGE_DEFAULT;
            TextureDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            TextureDesc.CPUAccessFlags      = 0;
            TextureDesc.MiscFlags           = D3D11_RESOURCE_MISC_GENERATE_MIPS;
            
            TextureDesc.SampleDesc.Count    = (MultiSamples_ > 0 ? MultiSamples_ : 1);
            TextureDesc.SampleDesc.Quality  = 0;
            
            /* Create the 2 dimensional texture */
            Result = D3DDevice_->CreateTexture2D(&TextureDesc, 0, &HWTexture2D_);
            D3DResource_ = HWTexture2D_;
        }
        break;
        
        case TEXTURE_3D:
        {
            /* Initialize texture description */
            D3D11_TEXTURE3D_DESC TextureDesc;
            
            TextureDesc.Width               = Size.X;
            TextureDesc.Height              = Size.Y;
            TextureDesc.Depth               = Size.Z;
            TextureDesc.MipLevels           = (MipMaps_ ? 0 : 1);
            TextureDesc.Format              = DxFormat;
            TextureDesc.Usage               = D3D11_USAGE_DEFAULT;
            TextureDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            TextureDesc.CPUAccessFlags      = 0;
            TextureDesc.MiscFlags           = D3D11_RESOURCE_MISC_GENERATE_MIPS;
            
            /* Create the 3 dimensional texture */
            Result = D3DDevice_->CreateTexture3D(&TextureDesc, 0, &HWTexture3D_);
            D3DResource_ = HWTexture3D_;
        }
        break;
        
        case TEXTURE_CUBEMAP:
        {
            /* Initialize texture description */
            D3D11_TEXTURE2D_DESC TextureDesc;
            
            TextureDesc.Width               = Size.X;
            TextureDesc.Height              = Size.Y;
            TextureDesc.MipLevels           = (MipMaps_ ? 0 : 1);
            TextureDesc.ArraySize           = 6;
            TextureDesc.Format              = DxFormat;
            TextureDesc.Usage               = D3D11_USAGE_DEFAULT;
            TextureDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            TextureDesc.CPUAccessFlags      = 0;
            TextureDesc.MiscFlags           = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
            
            TextureDesc.SampleDesc.Count    = (MultiSamples_ > 0 ? MultiSamples_ : 1);
            TextureDesc.SampleDesc.Quality  = 0;
            
            /* Create the 2 dimensional texture */
            Result = D3DDevice_->CreateTexture2D(&TextureDesc, 0, &HWTexture2D_);
            D3DResource_ = HWTexture2D_;
        }
        break;
    }
    
    /* Check if an error has been detected */
    if (Result)
    {
        io::Log::error("Could not create Direct3D11 texture");
        return false;
    }
    
    /* Update sampler state */
    updateSamplerState();
    
    /* Create shader resource view */
    if (D3DDevice_->CreateShaderResourceView(D3DResource_, 0, &ShaderResourceView_))
    {
        io::Log::error("Could not create shader resource view");
        return false;
    }
    
    if (isRenderTarget_)
        return updateRenderTarget();
    
    return true;
}

void Direct3D11Texture::updateTextureImage()
{
    if (ImageBuffer_->getBuffer())
    {
        const dim::size2di Size(ImageBuffer_->getSize());
        const u32 Pitch = ImageBuffer_->getPixelSize();
        
        D3DDeviceContext_->UpdateSubresource(
            D3DResource_, 0, 0, ImageBuffer_->getBuffer(),
            Pitch*Size.Width,
            Pitch*Size.Width*Size.Height
        );
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

bool Direct3D11Texture::updateRenderTarget()
{
    D3D11_RENDER_TARGET_VIEW_DESC* RenderTargetDesc = 0;
    
    /* Configure render target description for cube-maps */
    if (DimensionType_ == TEXTURE_CUBEMAP)
    {
        RenderTargetDesc = new D3D11_RENDER_TARGET_VIEW_DESC;
        
        setupTextureFormats(RenderTargetDesc->Format);
        
        RenderTargetDesc->ViewDimension                     = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        RenderTargetDesc->Texture2DArray.FirstArraySlice    = 0;
        RenderTargetDesc->Texture2DArray.ArraySize          = 6;
        RenderTargetDesc->Texture2DArray.MipSlice           = 0;
    }
    
    /* Create render target view */
    if (D3DDevice_->CreateRenderTargetView(D3DResource_, RenderTargetDesc, &RenderTargetView_))
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
            
            if (D3DDevice_->CreateRenderTargetView(D3DResource_, RenderTargetDesc, &RenderTargetViewCubeMap_[i]))
                io::Log::error("Could not create render target view for cube-map face #" + io::stringc(i));
        }
        
        delete RenderTargetDesc;
    }
    
    #if 1
    
    /* Setup depth texture description */
    D3D11_TEXTURE2D_DESC DepthTexDesc;
    
    const dim::size2di Size(ImageBuffer_->getSize());
    
    DepthTexDesc.Width              = Size.Width;
    DepthTexDesc.Height             = Size.Height;
    DepthTexDesc.MipLevels          = 1;//(MipMaps_ ? 0 : 1);
    DepthTexDesc.ArraySize          = 1;
    DepthTexDesc.Format             = DXGI_FORMAT_D32_FLOAT;
    DepthTexDesc.Usage              = D3D11_USAGE_DEFAULT;
    DepthTexDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
    DepthTexDesc.CPUAccessFlags     = 0;
    DepthTexDesc.MiscFlags          = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    
    DepthTexDesc.SampleDesc.Count   = 0;//(MultiSamples_ > 0 ? MultiSamples_ : 1);
    DepthTexDesc.SampleDesc.Quality = 0;
    
    /* Create depth texture */
    if (D3DDevice_->CreateTexture2D(&DepthTexDesc, 0, &DepthTexture_))
    {
        io::Log::error("Could not create Direct3D11 depth texture");
        return false;
    }
    
    /* Create depth stencil view */
    if (D3DDevice_->CreateDepthStencilView(DepthTexture_, 0, &DepthStencilView_))
    {
        io::Log::error("Could not create depth-stencil view");
        return false;
    }
    
    #endif
    
    #if 0
    
    if (Format_ == PIXELFORMAT_DEPTH)
    {
        /* Create depth stencil view */
        if (D3DDevice_->CreateDepthStencilView(D3DResource_, 0, &DepthStencilView_))
        {
            io::Log::error("Could not create depth stencil view");
            return false;
        }
    }
    
    #endif
    
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
