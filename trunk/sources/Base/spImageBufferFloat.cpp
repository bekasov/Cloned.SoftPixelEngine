/*
 * Image buffer float file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spImageBufferFloat.hpp"
#include "Base/spImageBufferUByte.hpp"


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

void ImageBufferFloat::copy(const ImageBuffer* Other)
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
            const ImageBufferUByte* UByteBuffer = static_cast<const ImageBufferUByte*>(Other);
            const u8* RawBuffer = static_cast<const u8*>(UByteBuffer->ImageBufferContainer<u8, 255>::getBuffer());
            
            for (u32 i = 0, c = getPixelCount()*getFormatSize(); i < c; ++i)
                Buffer_[i] = static_cast<f32>(RawBuffer[i]) / 255.0f;
        }
        break;
        
        case IMAGEBUFFER_FLOAT:
        {
            memcpy(Buffer_, Other->getBuffer(), getBufferSize());
        }
        break;
    }
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
