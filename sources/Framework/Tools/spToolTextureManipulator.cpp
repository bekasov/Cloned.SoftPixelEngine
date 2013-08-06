/*
 * Image modifier file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spToolTextureManipulator.hpp"

#ifdef SP_COMPILE_WITH_IMAGEMODIFIER


#include "RenderSystem/spTextureBase.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace tool
{


namespace ImageModifier
{

static void checkTexModDimension(video::ImageBuffer* ImgBuffer, dim::rect2di &Rect)
{
    if (Rect == DEF_TEXMANIP_RECT)
        Rect = dim::rect2di(0, 0, ImgBuffer->getSize().Width, ImgBuffer->getSize().Height);
    else
    {
        Rect.repair();
        
        const dim::size2di ImgSize(ImgBuffer->getSize());
        
        if (Rect.Right > ImgSize.Width)
            Rect.Right = ImgSize.Width;
        if (Rect.Bottom > ImgSize.Height)
            Rect.Bottom = ImgSize.Height;
    }
}

void drawMosaic(video::ImageBuffer* ImgBuffer, s32 PixelSize, dim::rect2di Rect)
{
    if (!ImgBuffer)
        return;
    
    checkTexModDimension(ImgBuffer, Rect);
    
    for (dim::point2di Pos(0, Rect.Top); Pos.Y < Rect.Bottom; ++Pos.Y)
    {
        for (Pos.X = Rect.Left; Pos.X < Rect.Right; ++Pos.X)
        {
            ImgBuffer->setPixelColor(
                Pos,
                ImgBuffer->getPixelColor(dim::point2di( ( Pos.X / PixelSize ) * PixelSize, ( Pos.Y / PixelSize ) * PixelSize ))
            );
        }
    }
}

void drawBlur(video::ImageBuffer* ImgBuffer, s32 PixelSize, dim::rect2di Rect)
{
    if (!ImgBuffer)
        return;
    
    checkTexModDimension(ImgBuffer, Rect);
    
    video::ImageBuffer* PrevImgBuffer = ImgBuffer->copy();
    
    /* Temporary variables */
    s32 sx, sy, w, h, c;
    video::color Color;
    s32 r, g, b;
    
    PixelSize /= 2;
    
    /* Loop for each line/ for each texel */
    for (dim::point2di Pos(0, Rect.Top); Pos.Y < Rect.Bottom; ++Pos.Y)
    {
        for (Pos.X = Rect.Left; Pos.X < Rect.Right; ++Pos.X)
        {
            /* Settings */
            r = g = b = 0;
            sy = Pos.Y - PixelSize >= 0 ? -PixelSize : -Pos.Y;
            w = Pos.X + PixelSize < Rect.Right ? PixelSize : Rect.Right - Pos.X;
            h = Pos.Y + PixelSize < Rect.Bottom ? PixelSize : Rect.Bottom - Pos.Y;
            
            /* Loop around the area of the current texel */
            for (c = 0; sy <= h; ++sy)
            {
                sx = Pos.X - PixelSize >= 0 ? -PixelSize : -Pos.X;
                
                for (; sx <= w; ++sx, ++c)
                {
                    Color = PrevImgBuffer->getPixelColor(Pos + dim::point2di(sx, sy));
                    r += Color.Red;
                    g += Color.Green;
                    b += Color.Blue;
                }
            }
            
            /* Set the new texel color */
            ImgBuffer->setPixelColor(Pos, video::color(r/c, g/c, b/c));
        }
    }
    
    delete PrevImgBuffer;
}

SP_EXPORT void bakeNormalMap(video::ImageBuffer* ImgBuffer, f32 Amplitude)
{
    /* Check if the texture it not empty */
    if (!ImgBuffer || ImgBuffer->getType() != video::IMAGEBUFFER_UBYTE)
        return;
    
    /* Make sure the texture has at least 3 color components */
    if (ImgBuffer->getFormatSize() < 3)
        ImgBuffer->setFormat(video::PIXELFORMAT_RGB);
    
    /* Temporary memory */
    dim::vector3df p1, p2, p3;
    dim::vector3df Normal;
    
    s32 Width   = ImgBuffer->getSize().Width;
    s32 Height  = ImgBuffer->getSize().Height;
    
    const s32 FormatSize = ImgBuffer->getFormatSize();
    const s32 ImageBufferSize = Width * Height * FormatSize;
    
    /* Copy the image data temporary */
    u8* PrevImgBuffer = MemoryManager::createBuffer<u8>(ImageBufferSize);
    memcpy(PrevImgBuffer, ImgBuffer->getBuffer(), ImageBufferSize);
    
    /* Loop for each texel */
    for (s32 y = 0, x; y < Height; ++y)
    {
        for (x = 0; x < Width; ++x)
        {
            /* Get the heights */
            p1.X = static_cast<f32>(x);
            p1.Y = static_cast<f32>(y);
            p1.Z = Amplitude * static_cast<f32>(PrevImgBuffer[(y * Width + x)*FormatSize]) / 255;
            
            p2.X = static_cast<f32>(x + 1);
            p2.Y = static_cast<f32>(y);
            
            if (x < Width - 1)
                p2.Z = Amplitude * static_cast<f32>(PrevImgBuffer[(y * Width + x + 1)*FormatSize]) / 255;
            else
                p2.Z = Amplitude * static_cast<f32>(PrevImgBuffer[(y * Width)*FormatSize]) / 255;
            
            p3.X = static_cast<f32>(x);
            p3.Y = static_cast<f32>(y + 1);
            
            if (y < Height - 1)
                p3.Z = Amplitude * static_cast<f32>(PrevImgBuffer[((y + 1) * Width + x)*FormatSize]) / 255;
            else
                p3.Z = Amplitude * static_cast<f32>(PrevImgBuffer[x*FormatSize]) / 255;
            
            /* Compute the normal */
            Normal = math::getNormalVector(p1, p2, p3);
            
            Normal *= 0.5f;
            Normal += 0.5f;
            Normal *= 255.0f;
            
            /* Set the new texel */
            ImgBuffer->setPixelColor(dim::point2di(x, y), video::color(Normal, false));
        }
    }
    
    delete [] PrevImgBuffer;
}

} // /namespace TextureModifier


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
