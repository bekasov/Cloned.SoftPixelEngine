/*
 * Image buffer ubyte header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIDEO_IMAGEBUFFER_UBYTE_H__
#define __SP_VIDEO_IMAGEBUFFER_UBYTE_H__


#include "Base/spStandard.hpp"
#include "Base/spImageBufferContainer.hpp"


namespace sp
{
namespace video
{


class ImageBufferUByte : public ImageBufferContainer<u8, 255>
{
    
    public:
        
        ImageBufferUByte();
        ImageBufferUByte(
            const EPixelFormats Format, const dim::size2di &Size, u32 Depth = 1, const u8* InitBuffer = 0
        );
        ImageBufferUByte(const ImageBufferUByte* Original);
        ~ImageBufferUByte();
        
        /* === Functions === */
        
        ImageBuffer* copy() const;
        void copy(const ImageBuffer* Other);
        
        void setBuffer(const void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size);
        void getBuffer(void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size) const;
        
        void setColorKey(const video::color &Color, u8 Tolerance);
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
