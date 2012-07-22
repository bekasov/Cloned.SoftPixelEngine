/*
 * Texture manipulator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spToolTextureManipulator.hpp"

#ifdef SP_COMPILE_WITH_TEXTUREMANIPULATOR


#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace tool
{


TextureManipulator::TextureManipulator()
{
}
TextureManipulator::~TextureManipulator()
{
}

void TextureManipulator::drawMosaic(video::Texture* Tex, s32 PixelSize, dim::rect2di Rect)
{
    if (!Tex)
        return;
    
    checkDimension(Tex, Rect);
    
    video::ImageBuffer* ImgBuffer = Tex->getImageBuffer();
    
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
    
    Tex->updateImageBuffer();
}

void TextureManipulator::drawBlur(video::Texture* Tex, s32 PixelSize, dim::rect2di Rect)
{
    if (!Tex)
        return;
    
    checkDimension(Tex, Rect);
    
    video::Texture* NewTexture = __spVideoDriver->copyTexture(Tex);
    
    /* Temporary variables */
    s32 sx, sy, w, h, c;
    video::color Color;
    s32 r, g, b;
    
    PixelSize /= 2;
    
    video::ImageBuffer* ImgBuffer = Tex->getImageBuffer();
    
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
                    Color = ImgBuffer->getPixelColor(Pos + dim::point2di(sx, sy));
                    r += Color.Red;
                    g += Color.Green;
                    b += Color.Blue;
                }
            }
            
            /* Set the new texel color */
            ImgBuffer->setPixelColor(Pos, video::color(r/c, g/c, b/c));
        }
    }
    
    Tex->updateImageBuffer();
    
    __spVideoDriver->deleteTexture(NewTexture);
}


/*
 * ======= Private: =======
 */

void TextureManipulator::checkDimension(const video::Texture* Tex, dim::rect2di &Rect)
{
    if (Rect == DEF_TEXMANIP_RECT)
        Rect = dim::rect2di(0, 0, Tex->getSize().Width, Tex->getSize().Height);
    else
    {
        Rect.repair();
        
        const dim::size2di TexSize(Tex->getSize());
        
        if (TexSize.Width < Rect.Right)
            Rect.Right = TexSize.Width;
        if (TexSize.Height < Rect.Bottom)
            Rect.Bottom = TexSize.Height;
    }
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
