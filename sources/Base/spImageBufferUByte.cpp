/*
 * Image buffer ubyte file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spImageBufferUByte.hpp"
#include "Base/spImageBufferFloat.hpp"

//#include <stdio.h>


namespace sp
{
namespace video
{


ImageBufferUByte::ImageBufferUByte() :
    ImageBufferContainer<u8, 255>(IMAGEBUFFER_UBYTE)
{
}
ImageBufferUByte::ImageBufferUByte(
    const EPixelFormats Format, const dim::size2di &Size, u32 Depth, const u8* InitBuffer) :
    ImageBufferContainer<u8, 255>(IMAGEBUFFER_UBYTE, Format, Size, Depth, InitBuffer)
{
}
ImageBufferUByte::ImageBufferUByte(const ImageBufferUByte* Original) :
    ImageBufferContainer<u8, 255>(
        IMAGEBUFFER_UBYTE, Original->getFormat(), Original->getSize(),
        Original->getDepth(), static_cast<const u8*>(Original->ImageBufferContainer<u8, 255>::getBuffer())
    )
{
}
ImageBufferUByte::~ImageBufferUByte()
{
}

ImageBuffer* ImageBufferUByte::copy() const
{
    return new ImageBufferUByte(this);
}

void ImageBufferUByte::copy(const ImageBuffer* Other)
{
    if (!Other)
        return;
    
    /* Copy base settings and resize if necessary */
    if (copyBase(*Other))
        createBuffer();
    
    /* Copy image buffer */
    switch (Other->getType())
    {
        case IMAGEBUFFER_UBYTE:
        {
            memcpy(Buffer_, Other->getBuffer(), getBufferSize());
        }
        break;
        
        case IMAGEBUFFER_FLOAT:
        {
            const ImageBufferFloat* FloatBuffer = static_cast<const ImageBufferFloat*>(Other);
            const f32* RawBuffer = static_cast<const f32*>(FloatBuffer->ImageBufferContainer<f32, 1>::getBuffer());
            
            for (u32 i = 0, c = getPixelCount()*getFormatSize(); i < c; ++i)
                Buffer_[i] = static_cast<u8>(math::MinMax(static_cast<s32>(RawBuffer[i] * 255.0f), 0, 255));
        }
        break;
    }
}

void ImageBufferUByte::setBuffer(const void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size)
{
    /* Create buffer if not already done */
    if (!Buffer_)
        createBuffer();
    
    /* Copy given buffer into this buffer */
    ImageConverter::copySubBufferToBuffer<u8>(
        Buffer_, static_cast<const u8*>(ImageBuffer), getSize(), getFormatSize(), Pos, Size
    );
}
void ImageBufferUByte::getBuffer(void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size) const
{
    ImageConverter::copyBufferToSubBuffer<u8>(
        static_cast<u8*>(ImageBuffer), Buffer_, getSize(), getFormatSize(), Pos, Size
    );
}

void ImageBufferUByte::setColorKey(const video::color &Color, u8 Tolerance)
{
    /* Check if color key is allowed */
    if (getFormat() == PIXELFORMAT_ALPHA || getFormat() == PIXELFORMAT_DEPTH)
        return;
    
    /* Check if color key is to be removed */
    if (Color.Alpha == 255)
    {
        switch (getFormat())
        {
            case PIXELFORMAT_GRAYALPHA:
                setFormat(PIXELFORMAT_GRAY);
                break;
            case PIXELFORMAT_RGBA:
                setFormat(PIXELFORMAT_RGB);
                break;
            case PIXELFORMAT_BGRA:
                setFormat(PIXELFORMAT_BGR);
                break;
        }
    }
    else
    {
        /* Check if pixel format must be adjusted */
        switch (getFormat())
        {
            case PIXELFORMAT_GRAY:
            case PIXELFORMAT_GRAYALPHA:
            case PIXELFORMAT_RGB:
                setFormat(PIXELFORMAT_RGBA);
                break;
            case PIXELFORMAT_BGR:
                setFormat(PIXELFORMAT_BGRA);
                break;
        }
        
        /* Convert image buffer for color key */
        ImageConverter::setImageColorKey(Buffer_, getSize().Width, getSize().Height, Color, Tolerance);
    }
    
    /* Store color key information */
    ColorKey_ = Color;
}


} // /namespace video

} // /namespace sp



// ================================================================================
