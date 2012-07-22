/*
 * Image buffer management file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spImageManagement.hpp"
#include "Base/spImageBuffer.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace video
{


/*
 * SUniversalImageData structure
 */

SHeightMapTexture::SHeightMapTexture() : ImageBuffer(0)
{
}
SHeightMapTexture::SHeightMapTexture(const Texture* Tex) : ImageBuffer(0)
{
    createBuffer(Tex);
}
SHeightMapTexture::~SHeightMapTexture()
{
}

void SHeightMapTexture::createBuffer(const Texture* Tex)
{
    if (!Tex)
        return;
    
    clearBuffer();
    
    Size = Tex->getSize();
    
    const video::ImageBuffer* ImgBuffer = Tex->getImageBuffer();
    
    const s32 ImageBufferSize = Size.Width * Size.Height;
    
    ImageBuffer = new f32[ImageBufferSize];
    memset(ImageBuffer, 0, ImageBufferSize*sizeof(f32));
    
    for (s32 y = 0, x, i = 0, j; y < Size.Height; ++y)
    {
        for (x = 0; x < Size.Width; ++x, ++i)
            ImageBuffer[i] = ImgBuffer->getPixelColor( dim::point2di(x, y) ).getBrightness<f32>() / 255;
    }
}

void SHeightMapTexture::createBuffer(const dim::size2di &NewSize, s32 NewFormat, const f32* NewImageBuffer)
{
    clearBuffer();
    
    Size = NewSize;
    
    const s32 ImageBufferSize = Size.Width * Size.Height;
    
    ImageBuffer = new f32[ImageBufferSize];
    memcpy(ImageBuffer, NewImageBuffer, ImageBufferSize);
}

void SHeightMapTexture::clearBuffer()
{
    Size = 0;
    MemoryManager::deleteBuffer(ImageBuffer);
}

f32 SHeightMapTexture::getHeightValue(const dim::point2df &Pos) const
{
    if (!ImageBuffer)
        return 0.0f;
    
    /* Get the first coordinate */
    s32 Pos1X = (s32)( (Pos.X - (s32)Pos.X) * Size.Width );
    s32 Pos1Y = (s32)( (Pos.Y - (s32)Pos.Y) * Size.Height );
    
    math::Clamp(Pos1X, 0, Size.Width - 1);
    math::Clamp(Pos1Y, 0, Size.Height - 1);
    
    /* Get the second coordinate */
    s32 Pos2X = Pos1X + 1;
    s32 Pos2Y = Pos1Y + 1;
    
    math::Clamp(Pos2X, 0, Size.Width - 1);
    math::Clamp(Pos2Y, 0, Size.Height - 1);
    
    /* Process the interpolation */
    f32 RatioX = Pos.X * Size.Width   - Pos1X;
    f32 RatioY = Pos.Y * Size.Height  - Pos1Y;
    
    math::Clamp(RatioX, 0.0f, 1.0f);
    math::Clamp(RatioY, 0.0f, 1.0f);
    
    const f32 RatioInvX = 1.0f - RatioX;
    const f32 RatioInvY = 1.0f - RatioY;
    
    /* Compute the final height value */
    const f32 val1 = ImageBuffer[ Pos1Y * Size.Width + Pos1X ];
    const f32 val2 = ImageBuffer[ Pos1Y * Size.Width + Pos2X ];
    const f32 val3 = ImageBuffer[ Pos2Y * Size.Width + Pos2X ];
    const f32 val4 = ImageBuffer[ Pos2Y * Size.Width + Pos1X ];
    
    return
        ( val1 * RatioInvX + val2 * RatioX ) * RatioInvY +
        ( val4 * RatioInvX + val3 * RatioX ) * RatioY;
}

dim::vector3df SHeightMapTexture::getNormal(const dim::point2df &Pos, const dim::point2df &Adjustment) const
{
    dim::vector3df a(Pos.X - Adjustment.X, 0, Pos.Y);
    dim::vector3df b(Pos.X + Adjustment.X, 0, Pos.Y);
    dim::vector3df c(Pos.X, 0, Pos.Y - Adjustment.Y);
    dim::vector3df d(Pos.X, 0, Pos.Y + Adjustment.Y);
    
    a.Y = getHeightValue(dim::point2df(a.X, a.Z));
    b.Y = getHeightValue(dim::point2df(b.X, b.Z));
    c.Y = getHeightValue(dim::point2df(c.X, c.Z));
    d.Y = getHeightValue(dim::point2df(d.X, d.Z));
    
    return (c - d).cross(a - b).normalize();
}


namespace ImageConverter
{

SP_EXPORT void setImageColorKey(u8* ImageBuffer, s32 Width, s32 Height, const video::color &Color, s32 Tolerance)
{
    if (!ImageBuffer || Width <= 0 || Height <= 0 || Tolerance < 0)
        return;
    
    /* Temporary memory */
    s32 r, g, b, i = 0;
    const s32 Cr = Color.Red, Cg = Color.Green, Cb = Color.Blue;
    
    for (s32 y = 0, x; y < Height; ++y)
    {
        for (x = 0; x < Width; ++x, i += 4)
        {   
            /* Get current color components */
            r = ImageBuffer[i + 0];
            g = ImageBuffer[i + 1];
            b = ImageBuffer[i + 2];
            
            /* Check color with tolerance */
            if ( ( Tolerance == 0 && Cr == r && Cg == g && Cb == b ) ||
                 ( Tolerance > 0 &&
                   Cr > r - Tolerance && Cr < r + Tolerance &&
                   Cg > g - Tolerance && Cg < g + Tolerance &&
                   Cb > b - Tolerance && Cb < b + Tolerance ) )
            {
                ImageBuffer[i + 3] = Color.Alpha;
            }
        }
    }
}

SP_EXPORT bool checkImageSize(dim::size2di &InputSize)
{
    dim::size2di OutputSize;
    
    /* Get the correct size */
    OutputSize.Width    = math::RoundPow2(InputSize.Width);
    OutputSize.Height   = math::RoundPow2(InputSize.Height);
    
    /* Check if the size has changed */
    if (OutputSize != InputSize)
    {
        InputSize = OutputSize;
        return true;
    }
    
    return false;
}

SP_EXPORT s32 getMipmapLevelsCount(s32 Width, s32 Height)
{
    return math::Round(
        log(math::Max<f32>(static_cast<f32>(Width), static_cast<f32>(Height))) / log(2.0f)
    );
}

} // /namespace ImageConverter


} // /namespace video

} // /namespace sp



// ================================================================================
