/*
 * Image buffer float file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spImageBufferFloat.hpp"


namespace sp
{
namespace video
{


ImageBufferFloat::ImageBufferFloat() :
    ImageBufferContainer<f32, 1>(IMAGEBUFFER_FLOAT)
{
}
ImageBufferFloat::ImageBufferFloat(
    const EPixelFormats Format, const dim::size2di &Size, u32 Depth, const f32* InitBuffer) :
    ImageBufferContainer<f32, 1>(IMAGEBUFFER_FLOAT, Format, Size, Depth, InitBuffer)
{
}
ImageBufferFloat::ImageBufferFloat(const ImageBufferFloat* Original) :
    ImageBufferContainer<f32, 1>(
        IMAGEBUFFER_FLOAT, Original->getFormat(), Original->getSize(),
        Original->getDepth(), static_cast<const f32*>(Original->ImageBufferContainer<f32, 1>::getBuffer()))
{
}
ImageBufferFloat::~ImageBufferFloat()
{
}

ImageBuffer* ImageBufferFloat::copy() const
{
    return new ImageBufferFloat(this);
}

void ImageBufferFloat::setBuffer(const void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size)
{
    /* Create buffer if not already done */
    if (!Buffer_)
        createBuffer();
    
    /* Copy given buffer into this buffer */
    ImageConverter::copySubBufferToBuffer<f32>(
        Buffer_, static_cast<const f32*>(ImageBuffer), getSize().Width, getFormatSize(), Pos, Size
    );
}
void ImageBufferFloat::getBuffer(void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size) const
{
    ImageConverter::copyBufferToSubBuffer<f32>(
        static_cast<f32*>(ImageBuffer), Buffer_, getSize(), getFormatSize(), Pos, Size
    );
}


} // /namespace video

} // /namespace sp



// ================================================================================
