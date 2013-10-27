/*
 * Direct3D11 texture file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D11/spDirect3D11Texture.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11TextureBuffer.hpp"
#include "Base/spImageManagement.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Framework/Tools/spUtilityDebugging.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


/*
 * Internal members
 */

const D3D11_TEXTURE_ADDRESS_MODE D3D11TextureWrapModes[] =
{
    D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_CLAMP,
};

const DXGI_FORMAT D3D11TexInternalFormatListUByte8[] =
{
    DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT
};

const DXGI_FORMAT D3D11TexInternalFormatListFloat16[] =
{
    DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT
};

const DXGI_FORMAT D3D11TexInternalFormatListFloat32[] =
{
    DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT
};

const DXGI_FORMAT D3D11TexInternalFormatListInt32[] =
{
    DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32B32_SINT,
    DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_D24_UNORM_S8_UINT
};

const DXGI_FORMAT D3D11TexInternalFormatListUInt32[] =
{
    DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT
};


/*
 * Direct3D11Texture class
 */

Direct3D11Texture::Direct3D11Texture(
    ID3D11Device* D3DDevice, ID3D11DeviceContext* D3DDeviceContext, const STextureCreationFlags &CreationFlags) :
    Texture             (CreationFlags      ),
    D3DDevice_          (D3DDevice          ),
    D3DDeviceContext_   (D3DDeviceContext   ),
    DepthTexture_       (0                  ),
    ResourceView_       (0                  ),
    AccessView_         (0                  ),
    RenderTargetView_   (0                  ),
    DepthStencilView_   (0                  ),
    SamplerSate_        (0                  ),
    TexBuffer_          (0                  )
{
    memset(RenderTargetViewCubeMap_, 0, sizeof(RenderTargetViewCubeMap_));
    
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
    return D3DResource_.Res != 0;
}

void Direct3D11Texture::setHardwareFormat(const EHWTextureFormats HardwareFormat)
{
    HWFormat_ = HardwareFormat;
    updateImageBuffer();
}

/* === Filter, MipMapping, Texture coordinate wraps === */

void Direct3D11Texture::setFilter(const STextureFilter &Filter)
{
    Texture::setFilter(Filter);
    updateSamplerState();
}

void Direct3D11Texture::setMinMagFilter(const ETextureFilters Filter)
{
    if (getMagFilter() != Filter || getMinFilter() != Filter)
    {
        Texture::setMinMagFilter(Filter);
        updateSamplerState();
    }
}
void Direct3D11Texture::setMinMagFilter(const ETextureFilters MagFilter, const ETextureFilters MinFilter)
{
    if (getMagFilter() != MagFilter || getMinFilter() != MinFilter)
    {
        Texture::setMinMagFilter(MagFilter, MinFilter);
        updateSamplerState();
    }
}
void Direct3D11Texture::setMagFilter(const ETextureFilters Filter)
{
    if (getMagFilter() != Filter)
    {
        Texture::setMagFilter(Filter);
        updateSamplerState();
    }
}
void Direct3D11Texture::setMinFilter(const ETextureFilters Filter)
{
    if (getMinFilter() != Filter)
    {
        Texture::setMinFilter(Filter);
        updateSamplerState();
    }
}

void Direct3D11Texture::setMipMapFilter(const ETextureMipMapFilters MipMapFilter)
{
    if (getMipMapFilter() != MipMapFilter)
    {
        Texture::setMipMapFilter(MipMapFilter);
        updateSamplerState();
    }
}

void Direct3D11Texture::setWrapMode(const ETextureWrapModes Wrap)
{
    if (getWrapMode().X != Wrap || getWrapMode().Y != Wrap || getWrapMode().Z != Wrap)
    {
        Texture::setWrapMode(Wrap);
        updateSamplerState();
    }
}
void Direct3D11Texture::setWrapMode(
    const ETextureWrapModes WrapU, const ETextureWrapModes WrapV, const ETextureWrapModes WrapW)
{
    if (getWrapMode().X != WrapU || getWrapMode().Y != WrapV || getWrapMode().Z != WrapW)
    {
        Texture::setWrapMode(WrapU, WrapV, WrapW);
        updateSamplerState();
    }
}


/* === Binding/ unbinding & image buffer === */

void Direct3D11Texture::bind(s32 Level) const
{
    D3D11_RENDER_SYS->setupShaderResourceView   (static_cast<u32>(Level), ResourceView_ );
    D3D11_RENDER_SYS->setupSamplerState         (static_cast<u32>(Level), SamplerSate_  );
}

void Direct3D11Texture::unbind(s32 Level) const
{
    D3D11_RENDER_SYS->setupShaderResourceView   (static_cast<u32>(Level), 0);
    D3D11_RENDER_SYS->setupSamplerState         (static_cast<u32>(Level), 0);
}

bool Direct3D11Texture::shareImageBuffer()
{
    /* Check for supported texture format */
    if (getHardwareFormat() != HWTEXFORMAT_FLOAT32 && getHardwareFormat() != HWTEXFORMAT_UBYTE8)
    {
        io::Log::error("Unsupported hardware texture format for CPU access (Only Float32 and UByte8 are supported)");
        return false;
    }
    
    /* Create texture with CPU access */
    UDx11TexResource D3DResource;
    if (!createHWTextureResource(D3DResource, ResourceViewPtr(), false, true, 0, 0))
        return false;
    
    /* Copy texture from original to temporary resource */
    D3DDeviceContext_->CopyResource(D3DResource.Res, D3DResource_.Res);
    
    /* Map temporary texture for reading from GPU to CPU */
    D3D11_MAPPED_SUBRESOURCE MappedRes;
    
    if (D3DDeviceContext_->Map(D3DResource.Res, 0, D3D11_MAP_READ, 0, &MappedRes) != S_OK)
    {
        io::Log::error("Mapping D3D11 texture resource failed");
        return false;
    }
    
    /* Copy data into image buffer */
    ImageBuffer_->setBuffer(MappedRes.pData);
    
    /* Release resource */
    D3DDeviceContext_->Unmap(D3DResource.Res, 0);
    Direct3D11RenderSystem::releaseObject(D3DResource.Res);
    
    return true;
}

bool Direct3D11Texture::updateImageBuffer()
{
    /* Re-create the hardware texture */
    if (!createHWTexture())
        return false;
    
    /* Refresh MRT view list for all referenced textures */
    refreshRefMRT();
    
    /* Update renderer image buffer */
    updateTextureImage();
    
    if (getMipMapping() && ResourceView_)
        D3DDeviceContext_->GenerateMips(ResourceView_);
    
    return true;
}


/*
 * ======= Private: =======
 */

void Direct3D11Texture::releaseResources()
{
    for (u32 i = 0; i < 6; ++i)
        Direct3D11RenderSystem::releaseObject(RenderTargetViewCubeMap_[i]);
    
    Direct3D11RenderSystem::releaseObject(D3DResource_.Res);
    
    Direct3D11RenderSystem::releaseObject(DepthTexture_     );
    
    Direct3D11RenderSystem::releaseObject(ResourceView_     );
    Direct3D11RenderSystem::releaseObject(AccessView_       );
    
    Direct3D11RenderSystem::releaseObject(RenderTargetView_ );
    Direct3D11RenderSystem::releaseObject(DepthStencilView_ );
    Direct3D11RenderSystem::releaseObject(SamplerSate_      );
    
    MemoryManager::deleteMemory(TexBuffer_);
}

bool Direct3D11Texture::createHWTextureResource(
    UDx11TexResource &D3DResource, ResourceViewPtr &ViewDesc,
    bool HasMIPMaps, bool HasCPUAccess, u32 BindFlags, u32 MiscFlags)
{
    /* Get DXGI format and texture size */
    const dim::vector3di Size(ImageBuffer_->getSizeVector());
    const DXGI_FORMAT DxFormat = getDxTexFormat();
    
    /* Setup CPU access flags and usage type */
    u32 AccessFlags = 0;
    D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
    
    if (HasCPUAccess)
    {
        AccessFlags |= D3D11_CPU_ACCESS_READ;
        Usage = D3D11_USAGE_STAGING;
    }
    
    /* Create a new Direct3D11 texture */
    HRESULT Result = 0;
    
    switch (getType())
    {
        case TEXTURE_1D:
        case TEXTURE_1D_ARRAY:
        case TEXTURE_1D_RW:
        case TEXTURE_1D_ARRAY_RW:
        {
            /* Initialize texture description */
            D3D11_TEXTURE1D_DESC TextureDesc;
            
            TextureDesc.Width               = Size.X;
            TextureDesc.MipLevels           = (HasMIPMaps ? 0 : 1);
            TextureDesc.ArraySize           = Size.Z;
            TextureDesc.Format              = DxFormat;
            TextureDesc.Usage               = Usage;
            TextureDesc.BindFlags           = BindFlags;
            TextureDesc.CPUAccessFlags      = AccessFlags;
            TextureDesc.MiscFlags           = MiscFlags;
            
            /* Create the 1 dimensional texture */
            Result = D3DDevice_->CreateTexture1D(&TextureDesc, 0, &D3DResource.Tex1D);
        }
        break;
        
        case TEXTURE_2D:
        case TEXTURE_2D_ARRAY:
        case TEXTURE_2D_RW:
        case TEXTURE_2D_ARRAY_RW:
        {
            /* Initialize texture description */
            D3D11_TEXTURE2D_DESC TextureDesc;
            
            TextureDesc.Width               = Size.X;
            TextureDesc.Height              = Size.Y;
            TextureDesc.MipLevels           = (HasMIPMaps ? 0 : 1);
            TextureDesc.ArraySize           = Size.Z;
            TextureDesc.Format              = DxFormat;
            TextureDesc.Usage               = Usage;
            TextureDesc.BindFlags           = BindFlags;
            TextureDesc.CPUAccessFlags      = AccessFlags;
            TextureDesc.MiscFlags           = MiscFlags;
            
            TextureDesc.SampleDesc.Count    = (MultiSamples_ > 0 ? MultiSamples_ : 1);
            TextureDesc.SampleDesc.Quality  = 0;
            
            /* Create the 2 dimensional texture */
            Result = D3DDevice_->CreateTexture2D(&TextureDesc, 0, &D3DResource.Tex2D);
        }
        break;
        
        case TEXTURE_3D:
        case TEXTURE_3D_RW:
        {
            /* Initialize texture description */
            D3D11_TEXTURE3D_DESC TextureDesc;
            
            TextureDesc.Width               = Size.X;
            TextureDesc.Height              = Size.Y;
            TextureDesc.Depth               = Size.Z;
            TextureDesc.MipLevels           = (HasMIPMaps ? 0 : 1);
            TextureDesc.Format              = DxFormat;
            TextureDesc.Usage               = Usage;
            TextureDesc.BindFlags           = BindFlags;
            TextureDesc.CPUAccessFlags      = AccessFlags;
            TextureDesc.MiscFlags           = MiscFlags;
            
            /* Create the 3 dimensional texture */
            Result = D3DDevice_->CreateTexture3D(&TextureDesc, 0, &D3DResource.Tex3D);
        }
        break;
        
        case TEXTURE_CUBEMAP:
        case TEXTURE_CUBEMAP_ARRAY:
        {
            /* Initialize texture description */
            D3D11_TEXTURE2D_DESC TextureDesc;
            
            TextureDesc.Width               = Size.X;
            TextureDesc.Height              = Size.Y;
            TextureDesc.MipLevels           = (HasMIPMaps ? 0 : 1);
            TextureDesc.ArraySize           = Size.Z * 6;
            TextureDesc.Format              = DxFormat;
            TextureDesc.Usage               = Usage;
            TextureDesc.BindFlags           = BindFlags;
            TextureDesc.CPUAccessFlags      = AccessFlags;
            TextureDesc.MiscFlags           = MiscFlags | D3D11_RESOURCE_MISC_TEXTURECUBE;
            
            TextureDesc.SampleDesc.Count    = (MultiSamples_ > 0 ? MultiSamples_ : 1);
            TextureDesc.SampleDesc.Quality  = 0;
            
            /* Create the 2 dimensional texture */
            Result = D3DDevice_->CreateTexture2D(&TextureDesc, 0, &D3DResource.Tex2D);
        }
        break;
        
        case TEXTURE_BUFFER:
        {
            /* Create D3D11 texture buffer */
            const u32 ElementNum    = Size.X*Size.Y*Size.Z;
            const u32 TexBufferSize = ElementNum * ImageBuffer_->getPixelSize();
            
            TexBuffer_ = new D3D11TextureBuffer(TexBufferSize);
            TexBuffer_->attachBuffer(ImageBuffer_->getBuffer());
            
            D3DResource.Res = TexBuffer_->getBufferRef();
            
            /* Setup resource view description */
            ViewDesc = ResourceViewPtr(new D3D11_SHADER_RESOURCE_VIEW_DESC);
            ZeroMemory(ViewDesc.get(), sizeof(ViewDesc));
            
            ViewDesc->Format                = DxFormat;
            ViewDesc->ViewDimension         = D3D11_SRV_DIMENSION_BUFFER;
            ViewDesc->Buffer.FirstElement   = 0;
            ViewDesc->Buffer.NumElements    = ElementNum;
        }
        break;
        
        default:
            io::Log::error("Unsupported texture dimension for Direct3D11 render system");
            return false;
    }
    
    /* Check if an error has been detected */
    if (Result)
    {
        io::Log::error("Could not create D3D11 texture (" + tool::Debugging::toString(getType()) + ")");
        return false;
    }
    
    return true;
}

bool Direct3D11Texture::createHWTexture()
{
    /* Delete old Direct3D11 resources */
    releaseResources();
    
    /* Adjust format size */
    ImageBuffer_->adjustFormatD3D();
    
    /* Initialize resource view description */
    ResourceViewPtr ViewDesc;
    
    /* Setup bind- and misc flags */
    u32 BindFlags = (!hasRWAccess() ? D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_UNORDERED_ACCESS);
    BindFlags |= D3D11_BIND_RENDER_TARGET;
    
    u32 MiscFlags = (!hasRWAccess() ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
    
    /* Create D3D11 hardware texture resource */
    if (!createHWTextureResource(D3DResource_, ViewDesc, getMipMapping(), false, BindFlags, MiscFlags))
        return false;
    
    /* Update sampler state */
    updateSamplerState();
    
    /* Create shader resource view or unordered access view */
    if (!hasRWAccess())
    {
        if (!createShaderResourceView(ViewDesc.get()))
            return false;
    }
    else
    {
        if (!createUnorderedAccessView())
            return false;
    }
    
    /* Create render target shader views */
    if (isRenderTarget_)
        return createRenderTargetViews();
    
    return true;
}

bool Direct3D11Texture::setupSubResourceData(D3D11_SUBRESOURCE_DATA &SubResourceData)
{
    if (ImageBuffer_->getBuffer() && !isRenderTarget_)
    {
        const dim::size2di Size(ImageBuffer_->getSize());
        const u32 Pitch = ImageBuffer_->getPixelSize();
        
        SubResourceData.pSysMem             = ImageBuffer_->getBuffer();
        SubResourceData.SysMemPitch         = Pitch*Size.Width;
        SubResourceData.SysMemSlicePitch    = Pitch*Size.Width*Size.Height;
        
        return true;
    }
    else
        ZeroMemory(&SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
    return false;
}

void Direct3D11Texture::updateTextureImage()
{
    D3D11_SUBRESOURCE_DATA SubResourceData;
    if (setupSubResourceData(SubResourceData))
    {
        D3DDeviceContext_->UpdateSubresource(
            D3DResource_.Res, 0, 0, SubResourceData.pSysMem,
            SubResourceData.SysMemPitch,
            SubResourceData.SysMemSlicePitch
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
    SamplerDesc.AddressU = D3D11TextureWrapModes[getWrapMode().X];
    SamplerDesc.AddressV = D3D11TextureWrapModes[getWrapMode().Y];
    SamplerDesc.AddressW = D3D11TextureWrapModes[getWrapMode().Z];
    
    /* Magnification, minification and MIP-mapping filter */
    if (getMipMapFilter() != FILTER_ANISOTROPIC)
    {
        if (getMagFilter() == FILTER_SMOOTH)
        {
            if (getMinFilter() == FILTER_SMOOTH)
            {
                if (getMipMapFilter() == FILTER_TRILINEAR)
                    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                else
                    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            }
            else
            {
                if (getMipMapFilter() == FILTER_TRILINEAR)
                    SamplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
                else
                    SamplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }
        }
        else
        {
            if (getMinFilter() == FILTER_SMOOTH)
            {
                if (getMipMapFilter() == FILTER_TRILINEAR)
                    SamplerDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                else
                    SamplerDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            }
            else
            {
                if (getMipMapFilter() == FILTER_TRILINEAR)
                    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                else
                    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            }
        }
    }
    else
        SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    
    /* Anisotropy */
    SamplerDesc.MaxAnisotropy   = (getMipMapFilter() == FILTER_ANISOTROPIC ? getAnisotropicSamples() : 0);
    
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

bool Direct3D11Texture::createRenderTargetViews()
{
    D3D11_RENDER_TARGET_VIEW_DESC* RenderTargetDesc = 0;
    
    /* Create render target view for cube-maps */
    switch (Type_)
    {
        case TEXTURE_CUBEMAP:
        {
            /* Configure render target description for cube-maps */
            D3D11_RENDER_TARGET_VIEW_DESC RenderTargetDesc;
            
            RenderTargetDesc.Format                         = getDxTexFormat();
            RenderTargetDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            RenderTargetDesc.Texture2DArray.FirstArraySlice = 0;
            RenderTargetDesc.Texture2DArray.ArraySize       = 6;
            RenderTargetDesc.Texture2DArray.MipSlice        = 0;
            RenderTargetDesc.Texture2DArray.ArraySize       = 1;
            
            for (u32 i = 0; i < 6; ++i)
            {
                RenderTargetDesc.Texture2DArray.FirstArraySlice = i;
                
                if (D3DDevice_->CreateRenderTargetView(D3DResource_.Res, &RenderTargetDesc, &RenderTargetViewCubeMap_[i]))
                {
                    io::Log::error("Could not create render target view for cube-map face #" + io::stringc(i));
                    return false;
                }
            }
        }
        break;
        
        default:
        {
            /* Create render target view */
            if (D3DDevice_->CreateRenderTargetView(D3DResource_.Res, 0, &RenderTargetView_))
            {
                io::Log::error("Could not create render target view");
                return false;
            }
        }
        break;
    }
    
    /* Create depth texture for render target */
    return createDepthTexture();
}

bool Direct3D11Texture::createDepthTexture()
{
    /* Create depth texture */
    D3D11_TEXTURE2D_DESC DepthTexDesc;
    ZeroMemory(&DepthTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
    
    const dim::size2di Size(ImageBuffer_->getSize());
    
    DepthTexDesc.Width              = Size.Width;
    DepthTexDesc.Height             = Size.Height;
    DepthTexDesc.MipLevels          = 1;
    DepthTexDesc.ArraySize          = 1;
    DepthTexDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthTexDesc.Usage              = D3D11_USAGE_DEFAULT;
    DepthTexDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
    DepthTexDesc.CPUAccessFlags     = 0;
    DepthTexDesc.MiscFlags          = 0;
    
    DepthTexDesc.SampleDesc.Count   = (MultiSamples_ > 0 ? MultiSamples_ : 1);
    DepthTexDesc.SampleDesc.Quality = 0;
    
    if (D3DDevice_->CreateTexture2D(&DepthTexDesc, 0, &DepthTexture_))
    {
        io::Log::error("Could not create Direct3D11 depth texture for render target");
        return false;
    }
    
    /* Create depth stencil view */
    if (D3DDevice_->CreateDepthStencilView(DepthTexture_, 0, &DepthStencilView_))
    {
        io::Log::error("Could not create depth-stencil view");
        return false;
    }
    
    return true;
}

void Direct3D11Texture::updateMultiRenderTargets()
{   
    /* Setup render target view list */
    MRTViewList_.resize(MRTList_.size() + 1);
    
    MRTViewList_[0] = RenderTargetView_;
    
    for (size_t i = 0, n = MRTList_.size(); i < n; ++i)
    {
        Direct3D11Texture* D3DTex = static_cast<Direct3D11Texture*>(MRTList_[i]);
        MRTViewList_[i + 1] = D3DTex->RenderTargetView_;
    }
}

void Direct3D11Texture::refreshRefMRT()
{
    /* Refresh MRT view list for all affected textures */
    foreach (Texture* Tex, MRTRefList_)
        static_cast<Direct3D11Texture*>(Tex)->updateMultiRenderTargets();
}

bool Direct3D11Texture::createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC* ViewDescRef)
{
    if (D3DDevice_->CreateShaderResourceView(D3DResource_.Res, ViewDescRef, &ResourceView_))
    {
        io::Log::error("Could not create shader resource view for D3D11 texture");
        return false;
    }
    return true;
}

bool Direct3D11Texture::createUnorderedAccessView()
{
    if (D3DDevice_->CreateUnorderedAccessView(D3DResource_.Res, 0, &AccessView_))
    {
        io::Log::error("Could not create unordered access view for D3D11 texture");
        return false;
    }
    return true;
}

DXGI_FORMAT Direct3D11Texture::getDxTexFormat() const
{
    const EPixelFormats Format = ImageBuffer_->getFormat();
    
    if (Format >= PIXELFORMAT_ALPHA && Format <= PIXELFORMAT_DEPTH)
    {
        switch (HWFormat_)
        {
            case HWTEXFORMAT_UBYTE8:    return D3D11TexInternalFormatListUByte8 [Format];
            case HWTEXFORMAT_FLOAT16:   return D3D11TexInternalFormatListFloat16[Format];
            case HWTEXFORMAT_FLOAT32:   return D3D11TexInternalFormatListFloat32[Format];
            case HWTEXFORMAT_INT32:     return D3D11TexInternalFormatListInt32  [Format];
            case HWTEXFORMAT_UINT32:    return D3D11TexInternalFormatListUInt32 [Format];
            default:                    break;
        }
    }
    
    return DXGI_FORMAT_UNKNOWN;
}

#undef D3D11_RENDER_SYS


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
