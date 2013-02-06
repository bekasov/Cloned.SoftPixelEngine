/*
 * Image buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spImageBuffer.hpp"


namespace sp
{
namespace video
{


ImageBuffer::ImageBuffer(const EImageBufferTypes Type) :
    Type_       (Type                               ),
    Format_     (PIXELFORMAT_RGB                    ),
    FormatSize_ (ImageBuffer::getFormatSize(Format_)),
    Depth_      (1                                  )
{
}
ImageBuffer::ImageBuffer(
    const EImageBufferTypes Type, const EPixelFormats Format, const dim::size2di &Size, u32 Depth) :
    Type_       (Type                                   ),
    Format_     (Format                                 ),
    FormatSize_ (ImageBuffer::getFormatSize(Format_)    ),
    Size_       (Size                                   ),
    Depth_      (math::Max(static_cast<u32>(1), Depth)  )
{
}
ImageBuffer::~ImageBuffer()
{
}

dim::point2di ImageBuffer::getPixelCoord(const dim::point2df &Pos) const
{
    return dim::point2di(
        static_cast<s32>( ( Pos.X - floor(Pos.X) ) * Size_.Width ),
        static_cast<s32>( ( Pos.Y - floor(Pos.Y) ) * Size_.Height )
    );
}

void ImageBuffer::setPixelColor(const s32 Pos, const color &Color)
{
    setPixelColor(dim::vector3di(Pos, 0, 0), Color);
}
void ImageBuffer::setPixelColor(const dim::point2di &Pos, const color &Color)
{
    setPixelColor(dim::vector3di(Pos.X, Pos.Y, 0), Color);
}
void ImageBuffer::setPixelColor(const dim::vector3di &Pos, const color &Color)
{
    if (Type_ == IMAGEBUFFER_UBYTE)
    {
        if (getFormatSize() > 2)
            setBuffer(&Color.Red, dim::point2di(Pos.X, Pos.Y + Pos.Z * Depth_), 1);
        else
        {
            const u8 Buf[2] = { Color.getBrightness<u8>(), Color.Alpha };
            setBuffer(Buf, dim::point2di(Pos.X, Pos.Y + Pos.Z * Depth_), 1);
        }
    }
}

void ImageBuffer::setPixelVector(const s32 Pos, const dim::vector4df &Color)
{
    setPixelVector(dim::vector3di(Pos, 0, 0), Color);
}
void ImageBuffer::setPixelVector(const dim::point2di &Pos, const dim::vector4df &Color)
{
    setPixelVector(dim::vector3di(Pos.X, Pos.Y, 0), Color);
}
void ImageBuffer::setPixelVector(const dim::vector3di &Pos, const dim::vector4df &Color)
{
    if (Type_ == IMAGEBUFFER_FLOAT)
    {
        if (getFormatSize() > 2)
            setBuffer(&Color.X, dim::point2di(Pos.X, Pos.Y + Pos.Z * Depth_), 1);
        else
        {
            const f32 Buf[2] = { (Color.X + Color.Y + Color.Z) / 3, Color.W };
            setBuffer(Buf, dim::point2di(Pos.X, Pos.Y + Pos.Z * Depth_), 1);
        }
    }
}

color ImageBuffer::getPixelColor(const s32 Pos) const
{
    return getPixelColor(dim::vector3di(Pos, 0, 0));
}
color ImageBuffer::getPixelColor(const dim::point2di &Pos) const
{
    return getPixelColor(dim::vector3di(Pos.X, Pos.Y, 0));
}
color ImageBuffer::getPixelColor(const dim::vector3di &Pos) const
{
    if (!Size_.getArea() || Type_ != IMAGEBUFFER_UBYTE)
        return color::empty;
    
    color Color;
    
    const s32 u = Pos.X % Size_.Width;
    const s32 v = (Pos.Y + Pos.Z * Depth_) % Size_.Height;
    
    getBuffer(&Color.Red, dim::point2di(u, v), 1);
    
    return Color;
}

dim::vector4df ImageBuffer::getPixelVector(const s32 Pos) const
{
    return getPixelVector(dim::vector3di(Pos, 0, 0));
}
dim::vector4df ImageBuffer::getPixelVector(const dim::point2di &Pos) const
{
    return getPixelVector(dim::vector3di(Pos.X, Pos.Y, 0));
}
dim::vector4df ImageBuffer::getPixelVector(const dim::vector3di &Pos) const
{
    if (!Size_.getArea() || Type_ != IMAGEBUFFER_FLOAT)
        return 0.0f;
    
    dim::vector4df Color;
    
    const s32 u = Pos.X % Size_.Width;
    const s32 v = (Pos.Y + Pos.Z * Depth_) % Size_.Height;
    
    getBuffer(&Color.X, dim::point2di(u, v), 1);
    
    return Color;
}

dim::vector4df ImageBuffer::getInterpolatedPixel(const dim::point2df &Pos) const
{
    /* Get all texture coordinates */
    dim::vector4df Texels[4];
    dim::point2di TexCoords[4];
    
    TexCoords[0] = getPixelCoord(Pos);
    TexCoords[1] = TexCoords[0];
    TexCoords[2] = TexCoords[0];
    TexCoords[3] = TexCoords[0];
    
    ++TexCoords[1].X;
    ++TexCoords[2].X;
    ++TexCoords[2].Y;
    ++TexCoords[3].Y;
    
    /* Get all surrounding texels */
    if (Type_ == IMAGEBUFFER_FLOAT)
    {
        for (s32 i = 0; i < 4; ++i)
            Texels[i] = getPixelVector(TexCoords[i]);
    }
    else
    {
        for (s32 i = 0; i < 4; ++i)
            getPixelColor(TexCoords[i]).getFloatArray(&Texels[i].X);
    }
    
    /* Interpolate between the four texels */
    const dim::point2df Interp(
        (Pos.X * Size_.Width ) - static_cast<f32>(TexCoords[0].X),
        (Pos.Y * Size_.Height) - static_cast<f32>(TexCoords[0].Y)
    );
    
    dim::vector4df Top, Bottom, Final;
    
    math::Lerp(Top, Texels[0], Texels[1], Interp.X);
    math::Lerp(Bottom, Texels[3], Texels[2], Interp.X);
    math::Lerp(Final, Top, Bottom, Interp.Y);
    
    return Final;
}

void ImageBuffer::setBuffer(const void* ImageBuffer)
{
    setBuffer(ImageBuffer, 0, getSize());
}

bool ImageBuffer::isSizePOT() const
{
    return getSize() == getSizePOT();
}
dim::size2di ImageBuffer::getSizePOT() const
{
    return dim::size2di(
        math::RoundPow2(Size_.Width),
        math::RoundPow2(Size_.Height)
    );
}
void ImageBuffer::setSizePOT()
{
    setSize(getSizePOT());
}

void ImageBuffer::adjustFormatD3D()
{
    switch (getFormat())
    {
        case PIXELFORMAT_RGB:
            setFormat(PIXELFORMAT_RGBA);
            break;
        case PIXELFORMAT_BGR:
            setFormat(PIXELFORMAT_BGRA);
            break;
        default:
            break;
    }
}

void ImageBuffer::setColorKey(const color &Color, u8 Tolerance)
{
    // do nothing
}
void ImageBuffer::setColorKey(const dim::point2di &Pos, u8 Alpha, u8 Tolerance)
{
    color Color(getPixelColor(Pos));
    Color.Alpha = Alpha;
    setColorKey(Color, Tolerance);
}
void ImageBuffer::setColorKeyAlpha(const EAlphaBlendingTypes Mode)
{
    // do nothing
}
void ImageBuffer::setColorKeyMask(ImageBuffer* MaskImage, const EAlphaBlendingTypes Mode)
{
    // do nothing
}

u32 ImageBuffer::getFormatSize(const EPixelFormats Format)
{
    switch (Format)
    {
        case PIXELFORMAT_ALPHA:
        case PIXELFORMAT_DEPTH:
        case PIXELFORMAT_GRAY:
            return 1;
        case PIXELFORMAT_GRAYALPHA:
            return 2;
        case PIXELFORMAT_RGB:
        case PIXELFORMAT_BGR:
            return 3;
        case PIXELFORMAT_RGBA:
        case PIXELFORMAT_BGRA:
            return 4;
        default:
            break;
    }
    return 0;
}

bool ImageBuffer::hasAlphaChannel(const EPixelFormats Format)
{
    switch (Format)
    {
        case PIXELFORMAT_ALPHA:
        case PIXELFORMAT_GRAYALPHA:
        case PIXELFORMAT_RGBA:
        case PIXELFORMAT_BGRA:
            return true;
        default:
            break;
    }
    return false;
}


/*
 * ======= Protected: =======
 */

bool ImageBuffer::copyBase(const ImageBuffer &Other)
{
    /* Check if something changed that the image buffer must be re-allocated */
    bool HasBufferChanged = (
        FormatSize_ != Other.FormatSize_ ||
        Size_ != Other.Size_ ||
        Depth_ != Other.Depth_
    );
    
    /* Copy base data */
    Format_     = Other.Format_;
    FormatSize_ = Other.FormatSize_;
    Size_       = Other.Size_;
    Depth_      = Other.Depth_;
    ColorKey_   = Other.ColorKey_;
    
    return HasBufferChanged;
}


} // /namespace video

} // /namespace sp



// ================================================================================
