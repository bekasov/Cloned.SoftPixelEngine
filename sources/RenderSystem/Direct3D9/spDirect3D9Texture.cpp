/*
 * Direct3D9 texture file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/Direct3D9/spDirect3D9Texture.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"
#include "Base/spImageManagement.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Framework/Tools/spUtilityDebugging.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


/*
 * Internal members
 */

static const c8* ERR_ONLY_UBYTE_BUFFERS = "Only UBYTE image buffers are supported for D3D9 textures";

static const s32 D3DTextureWrapModes[] =
{
    D3DTADDRESS_WRAP, D3DTADDRESS_MIRROR, D3DTADDRESS_CLAMP,
};

const D3DFORMAT D3DTexInternalFormatListUByte8[] =
{
    D3DFMT_A8, D3DFMT_L8, D3DFMT_A8L8, D3DFMT_X8R8G8B8,
    D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_D24X8
};

const D3DFORMAT D3DTexInternalFormatListFloat16[] =
{
    D3DFMT_R16F, D3DFMT_R16F, D3DFMT_G16R16F, D3DFMT_A16B16G16R16F,
    D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_A16B16G16R16F, D3DFMT_D24X8
};

const D3DFORMAT D3DTexInternalFormatListFloat32[] =
{
    D3DFMT_R32F, D3DFMT_R32F, D3DFMT_G32R32F, D3DFMT_A32B32G32R32F,
    D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_A32B32G32R32F, D3DFMT_D24X8
};


/*
 * Direct3D9Texture class
 */

Direct3D9Texture::Direct3D9Texture(const STextureCreationFlags &CreationFlags) :
    Texture(CreationFlags)
{
    ID_ = OrigID_ = this;

    if (CreationFlags.ImageBuffer)
        updateImageBuffer();
    else
        createHWTexture();
}
Direct3D9Texture::~Direct3D9Texture()
{
    if (D3DResource_.Res)
        D3DResource_.Res->Release();
}

bool Direct3D9Texture::valid() const
{
    return ImageBuffer_ != 0 && D3DResource_.Res != 0;
}

void Direct3D9Texture::bind(s32 Level) const
{
    updateTextureAttributes(Level);
    
    D3D9_DEVICE->SetTexture(Level, D3DResource_.Res);
}

void Direct3D9Texture::unbind(s32 Level) const
{
    D3D9_DEVICE->SetTexture(Level, 0);
}

bool Direct3D9Texture::shareImageBuffer()
{
    /* Check if the texture is 2 dimensional */
    if (Type_ != TEXTURE_2D || ImageBuffer_->getType() != IMAGEBUFFER_UBYTE)
        return false;
    
    /* Temporary variables */
    D3DLOCKED_RECT Rect;
    
    /* Lock the texture */
    if (D3DResource_.Tex2D->LockRect(0, &Rect, 0, D3DLOCK_READONLY))
    {
        io::Log::error("Could not lock Direct3D9 texture");
        return false;
    }
    
    /* Update the image buffer */
    const dim::size2di Size(getSize());
    
    readImageSurfaceBuffer(
        (const u8*)Rect.pBits,
        static_cast<u8*>(ImageBuffer_->getBuffer()),
        Size.Width, Size.Height
    );
    
    /* Unlock the texture */
    D3DResource_.Tex2D->UnlockRect(0);
    
    return true;
}

bool Direct3D9Texture::updateImageBuffer()
{
    /* ReCreate the image data */
    createHWTexture();
    
    if (ImageBuffer_ && !isRenderTarget_ && HWFormat_ == HWTEXFORMAT_UBYTE8)
    {
        if (is2D())
            updateImageTexture();
        else if (isCube())
        {
            for (s32 i = 0; i < 6; ++i)
                updateImageCubeTexture(i);
        }
        else if (isVolume())
            updateImageVolumeTexture();
    }
    
    return true;
}


/*
 * ======= Private: =======
 */

bool Direct3D9Texture::createHWTextureResource(
    bool MipMaps, const ETextureTypes Type, dim::vector3di Size, const EPixelFormats Format,
    const u8* ImageData, const EHWTextureFormats HWFormat, bool isRenderTarget)
{
    /* Get Direct3D9 device */
    IDirect3DDevice9* DxDevice = D3D9_DEVICE;

    /* Direct3D9 texture format setup */
    D3DFORMAT DxFormat  = D3DFMT_A8R8G8B8;
    DWORD Usage         = 0;
    D3DPOOL Pool        = D3DPOOL_MANAGED;

    setupTextureFormats(Format, HWFormat, DxFormat, Usage);
    
    D3DResource_.Res = 0;
    
    /* Setup usage flags */
    if (isRenderTarget)
    {
        Usage |= D3DUSAGE_RENDERTARGET;
        Pool = D3DPOOL_DEFAULT;
    }

    if (MipMaps)
        Usage |= D3DUSAGE_AUTOGENMIPMAP;
    
    /* Create a new D3D9 hardware texture */
    HRESULT Result = 0;
    
    switch (Type)
    {
        case TEXTURE_1D:
        {
            Result = DxDevice->CreateTexture(
                Size.X,
                1,
                MipMaps ? 0 : 1,
                Usage,
                DxFormat,
                Pool,
                &D3DResource_.Tex2D,
                0
            );
        }
        break;
        
        case TEXTURE_2D:
        {
            Result = DxDevice->CreateTexture(
                Size.X,
                Size.Y,
                MipMaps ? 0 : 1,
                Usage,
                DxFormat,
                Pool,
                &D3DResource_.Tex2D,
                0
            );
        }
        break;
        
        case TEXTURE_3D:
        {
            Result = DxDevice->CreateVolumeTexture(
                Size.X,
                Size.Y,
                Size.Z,
                MipMaps ? 0 : 1,
                Usage,
                DxFormat,
                Pool,
                &D3DResource_.TexVolume,
                0
            );
        }
        break;
        
        case TEXTURE_CUBEMAP:
        {
            Result = DxDevice->CreateCubeTexture(
                Size.X,
                MipMaps ? 0 : 1,
                Usage,
                DxFormat,
                Pool,
                &D3DResource_.TexCube,
                0
            );
        }
        break;
        
        default:
            io::Log::error("\"" + tool::Debugging::toString(Type) + "\" texture type is not supported for Direct3D 9 render system");
            return false;
    }
    
    /* Check if an error has been detected */
    if (Result != D3D_OK)
    {
        io::Log::error("Could not create Direct3D9 texture");
        return false;
    }
    
    return true;
}

bool Direct3D9Texture::createHWTexture()
{
    if (ImageBuffer_->getType() != IMAGEBUFFER_UBYTE)
    {
        io::Log::error(ERR_ONLY_UBYTE_BUFFERS);
        return false;
    }
    
    /* Adjust format size */
    ImageBuffer_->adjustFormatD3D();
    
    /* Delete the old Direct3D9 texture */
    Direct3D9RenderSystem::releaseObject(D3DResource_.Res);
    
    /* Create the new Direct3D9 texture */
    if (!createHWTextureResource(
            getMipMapping(),
            getType(),
            ImageBuffer_->getSizeVector(),
            ImageBuffer_->getFormat(),
            static_cast<const u8*>(ImageBuffer_->getBuffer()),
            getHardwareFormat(),
            getRenderTarget()))
    {
        return false;
    }
    
    return true;
}

void Direct3D9Texture::updateTextureAttributes(s32 SamplerLayer) const
{
    IDirect3DDevice9* DxDevice = D3D9_DEVICE;
    
    /* Wrap modes (reapeat, mirror, clamp) */
    DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_ADDRESSU, D3DTextureWrapModes[getWrapMode().X]);
    DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_ADDRESSV, D3DTextureWrapModes[getWrapMode().Y]);
    DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_ADDRESSW, D3DTextureWrapModes[getWrapMode().Z]);
    
    /* Anisotropy */
    DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_MAXANISOTROPY, getAnisotropicSamples());
    
    /* Texture filter */
    if (getMipMapping())
    {
        switch (getMipMapFilter())
        {
            case FILTER_BILINEAR:
                DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_MIPFILTER, D3DTEXF_POINT); break;
            case FILTER_TRILINEAR:
                DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR); break;
            case FILTER_ANISOTROPIC:
                DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC); break;
        }
    }
    else
        DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    
    /* Magnification filter */
    DxDevice->SetSamplerState(
        SamplerLayer, D3DSAMP_MAGFILTER,
        (getMagFilter() == FILTER_SMOOTH ? D3DTEXF_LINEAR : D3DTEXF_POINT)
    );
    
    /* Minification filter */
    D3DTEXTUREFILTERTYPE DxFilter = D3DTEXF_NONE;
    
    if (getMipMapFilter() == FILTER_ANISOTROPIC)
        DxFilter = D3DTEXF_ANISOTROPIC;
    else
        DxFilter = (getMinFilter() == FILTER_SMOOTH ? D3DTEXF_LINEAR : D3DTEXF_POINT);
    
    DxDevice->SetSamplerState(SamplerLayer, D3DSAMP_MINFILTER, DxFilter);
}

void Direct3D9Texture::updateImageTexture()
{
    if (ImageBuffer_->getType() != IMAGEBUFFER_UBYTE)
    {
        io::Log::error(ERR_ONLY_UBYTE_BUFFERS);
        return;
    }
    
    /* Temporary variables */
    D3DLOCKED_RECT Rect;
    
    const dim::size2di Size(getSize());
    
    /* Lock the texture */
    if (D3DResource_.Tex2D->LockRect(0, &Rect, 0, 0) != D3D_OK)
    {
        io::Log::error("Could not lock Direct3D9 texture");
        return;
    }
    
    if (Rect.Pitch / Size.Width != ImageBuffer_->getFormatSize())
    {
        io::Log::error("Software and hardware texture formats do not match");
        D3DResource_.Tex2D->UnlockRect(0);
        return;
    }
    
    /* Update the image data buffer */
    writeImageSurfaceBuffer(
        (u8*)Rect.pBits,
        static_cast<const u8*>(ImageBuffer_->getBuffer()),
        Size.Width, Size.Height
    );
    
    /* Unlock the texture */
    D3DResource_.Tex2D->UnlockRect(0);
    
    /* Check if mipmaps are used */
    #if 0
    if (isMipMapping_)
        createMipMaps();
    #endif
}

void Direct3D9Texture::updateImageCubeTexture(s32 Face)
{
    /* Temporary variables */
    D3DLOCKED_RECT Rect;
    
    const dim::size2di Size(getSize());
    const u32 Depth = ImageBuffer_->getDepth();
    
    const s32 SizeHeight        = (Depth > 1 ? Size.Height / Depth : Size.Height);
    const s32 ImageBufferSize   = Size.Width * SizeHeight * ImageBuffer_->getFormatSize();
    
    /* Lock the texture */
    if (D3DResource_.TexCube->LockRect((D3DCUBEMAP_FACES)Face, 0, &Rect, 0, 0) != D3D_OK)
    {
        io::Log::error("Could not lock Direct3D9 cubemap texture");
        return;
    }
    
    /* Update the image data buffer */
    writeImageSurfaceBuffer(
        (u8*)Rect.pBits,
        static_cast<const u8*>(ImageBuffer_->getBuffer()) + ImageBufferSize * Face,
        Size.Width, SizeHeight
    );
    
    /* Unlock the texture */
    D3DResource_.TexCube->UnlockRect((D3DCUBEMAP_FACES)Face, 0);
}

void Direct3D9Texture::updateImageVolumeTexture()
{
    if (ImageBuffer_->getType() != IMAGEBUFFER_UBYTE)
    {
        io::Log::error("Only UBYTE image buffer supported for D3D9 textures");
        return;
    }
    
    /* Temporary variables */
    D3DLOCKED_BOX Box;
    
    const dim::size2di Size(getSize());
    const u32 Depth = ImageBuffer_->getDepth();
    
    const s32 SizeHeight        = (Depth > 1 ? Size.Height / Depth : Size.Height);
    const s32 ImageBufferSize   = Size.Width * SizeHeight * ImageBuffer_->getFormatSize();
    
    /* Lock the texture */
    if (D3DResource_.TexVolume->LockBox(0, &Box, 0, 0) != D3D_OK)
    {
        io::Log::error("Could not lock Direct3D9 volume texture");
        return;
    }
    
    /* Update the image data buffer */
    for (u32 z = 0; z < Depth; ++z)
    {
        writeImageSurfaceBuffer(
            (u8*)Box.pBits + ImageBufferSize * z,
            static_cast<const u8*>(ImageBuffer_->getBuffer()) + ImageBufferSize * z,
            Size.Width, SizeHeight
        );
    }
    
    /* Unlock the texture */
    D3DResource_.TexVolume->UnlockBox(0);
}

void Direct3D9Texture::writeImageSurfaceBuffer(u8* DestImageBuffer, const u8* ImageBuffer, s32 Width, s32 Height)
{
    /* Check the incomming data buffer */
    if (!DestImageBuffer)
    {
        io::Log::error("Direct3D9 texture buffer is invalid");
        return;
    }
    
    /* Temporary varibales */
    s32 x, i = 0, c;
    
    /* Loop for the image data */
    switch (ImageBuffer_->getFormatSize())
    {
        case 1:
        {
            memcpy(DestImageBuffer, ImageBuffer, Width*Height);
        }
        break;
        
        case 2:
        {
            memcpy(DestImageBuffer, ImageBuffer, Width*Height*2);
        }
        break;
        
        case 3:
        {
            for (c = Width*Height*3; i < c; i += 3)
            {
                DestImageBuffer[i+0] = ImageBuffer[i+2];
                DestImageBuffer[i+1] = ImageBuffer[i+1];
                DestImageBuffer[i+2] = ImageBuffer[i+0];
            }
        }
        break;
        
        case 4:
        {
            for (c = Width*Height, x = 0; x < c; ++x, i += 4)
            {
                DestImageBuffer[i+0] = ImageBuffer[i+2];
                DestImageBuffer[i+1] = ImageBuffer[i+1];
                DestImageBuffer[i+2] = ImageBuffer[i+0];
                DestImageBuffer[i+3] = ImageBuffer[i+3];
            }
        }
        break;
    }
}

void Direct3D9Texture::readImageSurfaceBuffer(const u8* DestImageBuffer, u8* ImageBuffer, s32 Width, s32 Height)
{
    /* Check the incomming data buffer */
    if (!DestImageBuffer)
    {
        io::Log::error("Direct3D9 texture buffer is invalid");
        return;
    }
    
    /* Temporary varibales */
    s32 x, i = 0, c;
    
    /* Loop for the image data */
    switch (ImageBuffer_->getFormatSize())
    {
        case 1:
        {
            memcpy(ImageBuffer, DestImageBuffer, Width*Height);
        }
        break;
        
        case 2:
        {
            memcpy(ImageBuffer, DestImageBuffer, Width*Height*2);
        }
        break;
        
        case 3:
        {
            for (c = Width*Height*3; i < c; i += 3)
            {
                ImageBuffer[i+0] = DestImageBuffer[i+2];
                ImageBuffer[i+1] = DestImageBuffer[i+1];
                ImageBuffer[i+2] = DestImageBuffer[i+0];
            }
        }
        break;
        
        case 4:
        {
            for (c = Width*Height, x = 0; x < c; ++x, i += 4)
            {
                ImageBuffer[i+0] = DestImageBuffer[i+2];
                ImageBuffer[i+1] = DestImageBuffer[i+1];
                ImageBuffer[i+2] = DestImageBuffer[i+0];
                ImageBuffer[i+3] = DestImageBuffer[i+3];
            }
        }
        break;
    }
}


bool Direct3D9Texture::createMipMaps(s32 Level)
{
    /* Temporary variables */
    IDirect3DSurface9* UpperSurface = 0;
    IDirect3DSurface9* LowerSurface = 0;
    
    /* Get the upper level */
    if (D3DResource_.Tex2D->GetSurfaceLevel(Level - 1, &UpperSurface))
    {
        io::Log::error("Could not get the upper surface level");
        return false;
    }
    
    /* Get the lower level */
    if (D3DResource_.Tex2D->GetSurfaceLevel(Level, &LowerSurface))
    {
        io::Log::error("Could not get the lower surface level");
        return false;
    }
    
    /* Get the descriptions */
    D3DSURFACE_DESC UpperDesc, LowerDesc;
    
    UpperSurface->GetDesc(&UpperDesc);
    LowerSurface->GetDesc(&LowerDesc);
    
    /* Locked rectangles */
    D3DLOCKED_RECT UpperRect, LowerRect;
    
    /* Lock upper surface */
    if (UpperSurface->LockRect(&UpperRect, 0, 0))
    {
        io::Log::error("Could not lock upper texture rectangle");
        return false;
    }
    
    /* Lock lower surface */
    if (LowerSurface->LockRect(&LowerRect, 0, 0))
    {
        io::Log::error("Could not lock lower texture rectangle");
        return false;
    }
    
    /* Generate the current mip map level */
    generateMipMapLevel((s32*)UpperRect.pBits, (s32*)LowerRect.pBits, LowerDesc.Width, LowerDesc.Height);
    
    /* Unlock surfaces */
    if (UpperSurface->UnlockRect())
        return false;
    if (LowerSurface->UnlockRect())
        return false;
    
    /* Release the surfaces */
    UpperSurface->Release();
    LowerSurface->Release();
    
    /* Check if the mip map generation has been finished */
    if (LowerDesc.Width == 1 && LowerDesc.Height == 1)
        return true;
    
    /* Generate the next mip map level */
    return createMipMaps(Level + 1);
}

void Direct3D9Texture::generateMipMapLevel(s32* src, s32* dst, s32 Width, s32 Height)
{
    /* Temporary variables */
    s32 x, y, px, py;
    s32 r, g, b, a;
    s32 i, j;
    
    /* Loop for the new mipmap texture */
    for (y = 0; y < Height; ++y)
    {
        for (x = 0; x < Width; ++x)
        {
            /* Initialize the color values */
            r = g = b = a = 0;
            
            /* Loop for the old pixel area */
            for (py = 0; py < 2; ++py)
            {
                for (px = 0; px < 2; ++px)
                {
                    /* Compute the source index */
                    i = ((y << 1) + py) * (Width << 1) + (x << 1) + px;
                    
                    /* Add the color values */
                    r += getRed(src[i]);
                    g += getGreen(src[i]);
                    b += getBlue(src[i]);
                    a += getAlpha(src[i]);
                }
            }
            
            /* Divide the color values */
            r >>= 2;
            g >>= 2;
            b >>= 2;
            a >>= 2;
            
            /* Compute the destination index */
            j = y * Width + x;
            
            /* Set the new color value */
            dst[j] = video::color(r, g, b, a).getSingle();
        }
    }
}

bool Direct3D9Texture::is2D() const
{
    return Type_ == TEXTURE_1D || Type_ == TEXTURE_2D;
}
bool Direct3D9Texture::isCube() const
{
    return Type_ == TEXTURE_3D;
}
bool Direct3D9Texture::isVolume() const
{
    return Type_ == TEXTURE_CUBEMAP;
}

void Direct3D9Texture::setupTextureFormats(
    const EPixelFormats Format, const EHWTextureFormats HWFormat, D3DFORMAT &DxFormat, DWORD &Usage)
{
    if (Format >= PIXELFORMAT_ALPHA && Format <= PIXELFORMAT_DEPTH)
    {
        switch (HWFormat)
        {
            case HWTEXFORMAT_UBYTE8:
                DxFormat = D3DTexInternalFormatListUByte8[Format]; break;
            case HWTEXFORMAT_FLOAT16:
                DxFormat = D3DTexInternalFormatListFloat16[Format]; break;
            case HWTEXFORMAT_FLOAT32:
                DxFormat = D3DTexInternalFormatListFloat32[Format]; break;
        }
    }
    
    switch (Format)
    {
        case PIXELFORMAT_DEPTH:
            Usage = D3DUSAGE_DEPTHSTENCIL; break;
        default:
            break;
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
