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


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


static const s32 D3DTextureWrapModes[] =
{
    D3DTADDRESS_WRAP, D3DTADDRESS_MIRROR, D3DTADDRESS_CLAMP,
};

Direct3D9Texture::Direct3D9Texture() :
    Texture             (   ),
    D3DBaseTexture_     (0  ),
    D3D2DTexture_       (0  ),
    D3DCubeTexture_     (0  ),
    D3DVolumeTexture_   (0  )
{
}
Direct3D9Texture::Direct3D9Texture(
    IDirect3DTexture9* d3dTexture, IDirect3DCubeTexture9* d3dCubeTexture,
    IDirect3DVolumeTexture9* d3dVolumeTexture, const STextureCreationFlags &CreationFlags) :
    Texture             (CreationFlags      ),
    D3DBaseTexture_     (0                  ),
    D3D2DTexture_       (d3dTexture         ),
    D3DCubeTexture_     (d3dCubeTexture     ),
    D3DVolumeTexture_   (d3dVolumeTexture   )
{
    ID_ = OrigID_ = this;
    
    updateBaseTexture();
    
    if (CreationFlags.ImageBuffer)
        updateImageBuffer();
}
Direct3D9Texture::~Direct3D9Texture()
{
    clear();
}

bool Direct3D9Texture::valid() const
{
    return ImageBuffer_ && D3DBaseTexture_;
}

void Direct3D9Texture::bind(s32 Level) const
{
    updateTextureAttributes(Level);
    
    static_cast<Direct3D9RenderSystem*>(GlbRenderSys)->getDirect3DDevice()->SetTexture(Level, D3DBaseTexture_);
}

void Direct3D9Texture::unbind(s32 Level) const
{
    static_cast<Direct3D9RenderSystem*>(GlbRenderSys)->getDirect3DDevice()->SetTexture(Level, 0);
}

bool Direct3D9Texture::shareImageBuffer()
{
    /* Check if the texture is 2 dimensional */
    if (Type_ != TEXTURE_2D || ImageBuffer_->getType() != IMAGEBUFFER_UBYTE)
        return false;
    
    /* Temporary variables */
    D3DLOCKED_RECT Rect;
    
    /* Lock the texture */
    if (D3D2DTexture_->LockRect(0, &Rect, 0, D3DLOCK_READONLY))
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
    D3D2DTexture_->UnlockRect(0);
    
    return true;
}

bool Direct3D9Texture::updateImageBuffer()
{
    /* Clear the image data */
    recreateHWTexture();
    
    if (ImageBuffer_ && !isRenderTarget_ && HWFormat_ == HWTEXFORMAT_UBYTE8)
    {
        if (D3D2DTexture_)
            updateImageTexture();
        else if (D3DCubeTexture_)
        {
            for (s32 i = 0; i < 6; ++i)
                updateImageCubeTexture(i);
        }
        else if (D3DVolumeTexture_)
            updateImageVolumeTexture();
    }
    
    return true;
}


/*
 * ======= Private: =======
 */

void Direct3D9Texture::clear()
{
    updateBaseTexture();
    
    if (D3DBaseTexture_)
        D3DBaseTexture_->Release();
    
    D3D2DTexture_       = 0;
    D3DCubeTexture_     = 0;
    D3DVolumeTexture_   = 0;
}

void Direct3D9Texture::updateTextureAttributes(s32 SamplerLayer) const
{
    IDirect3DDevice9* DxDevice = static_cast<Direct3D9RenderSystem*>(GlbRenderSys)->getDirect3DDevice();
    
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

void Direct3D9Texture::recreateHWTexture()
{
    if (ImageBuffer_->getType() != IMAGEBUFFER_UBYTE)
    {
        io::Log::error("Only UBYTE image buffer supported for D3D9 textures");
        return;
    }
    
    /* Adjust format size */
    ImageBuffer_->adjustFormatD3D();
    
    /* Delete the old Direct3D9 texture */
    clear();
    
    Direct3D9RenderSystem* D3DRenderer = static_cast<Direct3D9RenderSystem*>(GlbRenderSys);
    
    /* Create the new Direct3D9 texture */
    D3DRenderer->createRendererTexture(
        getMipMapping(),
        getType(),
        ImageBuffer_->getSizeVector(),
        ImageBuffer_->getFormat(),
        static_cast<const u8*>(ImageBuffer_->getBuffer()),
        getHardwareFormat(),
        getRenderTarget()
    );
    
    if (D3DRenderer->CurD3DTexture_)
        D3D2DTexture_ = D3DRenderer->CurD3DTexture_;
    else if (D3DRenderer->CurD3DCubeTexture_)
        D3DCubeTexture_ = D3DRenderer->CurD3DCubeTexture_;
    else if (D3DRenderer->CurD3DVolumeTexture_)
        D3DVolumeTexture_ = D3DRenderer->CurD3DVolumeTexture_;
    
    updateBaseTexture();
}


void Direct3D9Texture::updateImageTexture()
{
    if (ImageBuffer_->getType() != IMAGEBUFFER_UBYTE)
    {
        io::Log::error("Only UBYTE image buffer supported for D3D9 textures");
        return;
    }
    
    /* Temporary variables */
    D3DLOCKED_RECT Rect;
    
    const dim::size2di Size(getSize());
    
    /* Lock the texture */
    if (D3D2DTexture_->LockRect(0, &Rect, 0, 0))
    {
        io::Log::error("Could not lock Direct3D9 texture");
        return;
    }
    
    if (Rect.Pitch / Size.Width != ImageBuffer_->getFormatSize())
    {
        io::Log::error("Software and hardware texture formats do not match");
        D3D2DTexture_->UnlockRect(0);
        return;
    }
    
    /* Update the image data buffer */
    writeImageSurfaceBuffer(
        (u8*)Rect.pBits,
        static_cast<const u8*>(ImageBuffer_->getBuffer()),
        Size.Width, Size.Height
    );
    
    /* Unlock the texture */
    D3D2DTexture_->UnlockRect(0);
    
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
    if (D3DCubeTexture_->LockRect((D3DCUBEMAP_FACES)Face, 0, &Rect, 0, 0))
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
    D3DCubeTexture_->UnlockRect((D3DCUBEMAP_FACES)Face, 0);
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
    if (D3DVolumeTexture_->LockBox(0, &Box, 0, 0))
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
    D3DVolumeTexture_->UnlockBox(0);
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
    if (D3D2DTexture_->GetSurfaceLevel(Level - 1, &UpperSurface))
    {
        io::Log::error("Could not get the upper surface level");
        return false;
    }
    
    /* Get the lower level */
    if (D3D2DTexture_->GetSurfaceLevel(Level, &LowerSurface))
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

void Direct3D9Texture::updateBaseTexture()
{
    if (D3D2DTexture_)
        D3DBaseTexture_ = D3D2DTexture_;
    else if (D3DCubeTexture_)
        D3DBaseTexture_ = D3DCubeTexture_;
    else if (D3DVolumeTexture_)
        D3DBaseTexture_ = D3DVolumeTexture_;
    else
        D3DBaseTexture_ = 0;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
