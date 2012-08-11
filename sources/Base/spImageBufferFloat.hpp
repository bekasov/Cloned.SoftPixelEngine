/*
 * Image buffer float header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIDEO_IMAGEBUFFER_FLOAT_H__
#define __SP_VIDEO_IMAGEBUFFER_FLOAT_H__


#include "Base/spStandard.hpp"
#include "Base/spImageBufferContainer.hpp"


namespace sp
{
namespace video
{


class ImageBufferFloat : public ImageBufferContainer<f32, 1>
{
    
    public:
        
        ImageBufferFloat();
        ImageBufferFloat(
            const EPixelFormats Format, const dim::size2di &Size, u32 Depth = 1, const f32* InitBuffer = 0
        );
        ImageBufferFloat(const ImageBufferFloat* Original);
        ~ImageBufferFloat();
        
        /* === Functions === */
        
        ImageBuffer* copy() const;
        void copy(const ImageBuffer* Other);
        
        void setBuffer(const void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size);
        void getBuffer(void* ImageBuffer, const dim::point2di &Pos, const dim::size2di &Size) const;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
