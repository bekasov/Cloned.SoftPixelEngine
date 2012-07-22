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

extern video::RenderSystem* __spVideoDriver;

namespace video
{


/*
 * Internal members
 */

s32 D3D9PixelFormatDataSize[] = {
    1, 1, 2, 4, 4, 4, 4, 1
};


/*
 * Direct3D9Texture class
 */

Direct3D9Texture::Direct3D9Texture()
    : Texture(), pDirect3DTexture_(0), pDirect3DCubeTexture_(0), pDirect3DVolumeTexture_(0)
{
}
Direct3D9Texture::Direct3D9Texture(
    IDirect3DTexture9* d3dTexture, IDirect3DCubeTexture9* d3dCubeTexture,
    IDirect3DVolumeTexture9* d3dVolumeTexture, const STextureCreationFlags &CreationFlags)
    : Texture(CreationFlags), pDirect3DTexture_(d3dTexture),
    pDirect3DCubeTexture_(d3dCubeTexture), pDirect3DVolumeTexture_(d3dVolumeTexture)
{
    ID_ = OrigID_ = this;
    
    updateBaseTexture();
    
    if (CreationFlags.ImageBuffer)
        Texture::updateImageBuffer(CreationFlags.ImageBuffer);
}
Direct3D9Texture::~Direct3D9Texture()
{
    clear();
}

bool Direct3D9Texture::valid() const
{
    return ImageBuffer_ && pDirect3DBaseTexture_;
}

void Direct3D9Texture::setColorIntensity(f32 Red, f32 Green, f32 Blue)
{
    // !TODO!
}

void Direct3D9Texture::bind(s32 Level) const
{
    Direct3D9Texture* Tex = static_cast<Direct3D9Texture*>(ID_);
    
    static_cast<Direct3D9RenderSystem*>(__spVideoDriver)->CurSamplerLevel_ = Level;
    
    static_cast<Direct3D9RenderSystem*>(__spVideoDriver)->updateTextureAttributes(
        Tex->DimensionType_, Tex->MagFilter_, Tex->MinFilter_, Tex->MipMapFilter_,
        Tex->AnisotropicSamples_, Tex->MipMaps_, Tex->WrapMode_
    );
    
    static_cast<Direct3D9RenderSystem*>(__spVideoDriver)->getDirect3DDevice()->SetTexture(Level, Tex->pDirect3DBaseTexture_);
}

void Direct3D9Texture::unbind(s32 Level) const
{
    static_cast<Direct3D9RenderSystem*>(__spVideoDriver)->getDirect3DDevice()->SetTexture(Level, 0);
}

void Direct3D9Texture::shareImageBuffer()
{
    /* Check if the texture is 2 dimensional */
    if (DimensionType_ != TEXTURE_2D)
        return;
    
    /* Temporary variables */
    D3DLOCKED_RECT Rect;
    
    /* Lock the texture */
    if (pDirect3DTexture_->LockRect(0, &Rect, 0, D3DLOCK_READONLY))
    {
        io::Log::error("Could not lock Direct3D9 texture");
        return;
    }
    
    /* Update the image buffer */
    readImageSurfaceBuffer((const u8*)Rect.pBits, ImageBuffer_, Size_.Width, Size_.Height);
    
    /* Unlock the texture */
    pDirect3DTexture_->UnlockRect(0);
}

void Direct3D9Texture::updateImageBuffer()
{
    /* Update renderer texture format */
    updateFormat();
    
    /* Clear the image data */
    recreateHWTexture();
    
    if (ImageBuffer_ && !isRenderTarget_ && HWFormat_ == HWTEXFORMAT_UBYTE8)
    {
        if (pDirect3DTexture_)
            updateImageTexture();
        else if (pDirect3DCubeTexture_)
        {
            for (s32 i = 0; i < 6; ++i)
                updateImageCubeTexture(i);
        }
        else if (pDirect3DVolumeTexture_)
            updateImageVolumeTexture();
    }
}


/*
 * ======= Private: =======
 */

void Direct3D9Texture::clear()
{
    updateBaseTexture();
    
    if (pDirect3DBaseTexture_)
        pDirect3DBaseTexture_->Release();
    
    pDirect3DTexture_       = 0;
    pDirect3DCubeTexture_   = 0;
    pDirect3DVolumeTexture_ = 0;
}

void Direct3D9Texture::updateFormat()
{
    /* Update image buffer size */
    if (getFormatSize(Format_) != FormatSize_)
        ImageConverter::convertImageFormat(ImageBuffer_, Size_.Width, Size_.Height, FormatSize_, getFormatSize(Format_));
    
    /* Update only format size */
    updateFormatSize();
}

void Direct3D9Texture::updateFormatSize()
{
    FormatSize_ = D3D9PixelFormatDataSize[Format_];
}
s32 Direct3D9Texture::getFormatSize(const EPixelFormats Format) const
{
    return D3D9PixelFormatDataSize[Format];
}

void Direct3D9Texture::recreateHWTexture()
{
    /* Delete the old Direct3D9 texture */
    clear();
    
    Direct3D9RenderSystem* d3dDriver = static_cast<Direct3D9RenderSystem*>(__spVideoDriver);
    
    /* Create the new Direct3D9 texture */
    d3dDriver->createRendererTexture(
        MipMaps_, DimensionType_, dim::vector3di(Size_.Width, Size_.Height, Depth_),
        Format_, ImageBuffer_, HWFormat_, isRenderTarget_
    );
    
    if (d3dDriver->CurD3DTexture_)
        pDirect3DTexture_       = d3dDriver->CurD3DTexture_;
    else if (d3dDriver->CurD3DCubeTexture_)
        pDirect3DCubeTexture_   = d3dDriver->CurD3DCubeTexture_;
    else if (d3dDriver->CurD3DVolumeTexture_)
        pDirect3DVolumeTexture_ = d3dDriver->CurD3DVolumeTexture_;
    
    updateBaseTexture();
}


void Direct3D9Texture::updateImageTexture()
{
    /* Temporary variables */
    D3DLOCKED_RECT Rect;
    
    /* Lock the texture */
    if (pDirect3DTexture_->LockRect(0, &Rect, 0, 0))
    {
        io::Log::error("Could not lock Direct3D9 texture");
        return;
    }
    
    if (Rect.Pitch / Size_.Width != FormatSize_)
    {
        io::Log::error("Software and hardware texture formats do not match");
        pDirect3DTexture_->UnlockRect(0);
        return;
    }
    
    /* Update the image data buffer */
    writeImageSurfaceBuffer((u8*)Rect.pBits, ImageBuffer_, Size_.Width, Size_.Height);
    
    /* Unlock the texture */
    pDirect3DTexture_->UnlockRect(0);
    
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
    
    const s32 SizeHeight        = (Depth_ > 1 ? Size_.Height / Depth_ : Size_.Height);
    const s32 ImageBufferSize   = Size_.Width * SizeHeight * FormatSize_;
    
    /* Lock the texture */
    if (pDirect3DCubeTexture_->LockRect((D3DCUBEMAP_FACES)Face, 0, &Rect, 0, 0))
    {
        io::Log::error("Could not lock Direct3D9 cubemap texture");
        return;
    }
    
    /* Update the image data buffer */
    writeImageSurfaceBuffer((u8*)Rect.pBits, ImageBuffer_ + ImageBufferSize * Face, Size_.Width, SizeHeight);
    
    /* Unlock the texture */
    pDirect3DCubeTexture_->UnlockRect((D3DCUBEMAP_FACES)Face, 0);
}

void Direct3D9Texture::updateImageVolumeTexture()
{
    /* Temporary variables */
    D3DLOCKED_BOX Box;
    
    const s32 SizeHeight        = (Depth_ > 1 ? Size_.Height / Depth_ : Size_.Height);
    const s32 ImageBufferSize   = Size_.Width * SizeHeight * FormatSize_;
    
    /* Lock the texture */
    if (pDirect3DVolumeTexture_->LockBox(0, &Box, 0, 0))
    {
        io::Log::error("Could not lock Direct3D9 volume texture");
        return;
    }
    
    /* Update the image data buffer */
    for (s32 z = 0; z < Depth_; ++z)
        writeImageSurfaceBuffer((u8*)Box.pBits + ImageBufferSize * z, ImageBuffer_ + ImageBufferSize * z, Size_.Width, SizeHeight);
    
    /* Unlock the texture */
    pDirect3DVolumeTexture_->UnlockBox(0);
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
    s32 x, y, i = 0, c;
    
    /* Loop for the image data */
    switch (FormatSize_)
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
    s32 x, y, i = 0, c;
    
    /* Loop for the image data */
    switch (FormatSize_)
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
    if (pDirect3DTexture_->GetSurfaceLevel(Level - 1, &UpperSurface))
    {
        io::Log::error("Could not get the upper surface level");
        return false;
    }
    
    /* Get the lower level */
    if (pDirect3DTexture_->GetSurfaceLevel(Level, &LowerSurface))
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
    if (pDirect3DTexture_)
        pDirect3DBaseTexture_ = pDirect3DTexture_;
    else if (pDirect3DCubeTexture_)
        pDirect3DBaseTexture_ = pDirect3DCubeTexture_;
    else if (pDirect3DVolumeTexture_)
        pDirect3DBaseTexture_ = pDirect3DVolumeTexture_;
    else
        pDirect3DBaseTexture_ = 0;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
