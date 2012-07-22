/*
 * Image loader PNG header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGELOADER_PNG_H__
#define __SP_IMAGELOADER_PNG_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_PNG


#include "FileFormats/Image/spImageFormatInterfaces.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT ImageLoaderPNG : public ImageLoader
{
    
    public:
        
        ImageLoaderPNG(io::File* File);
        ~ImageLoaderPNG();
        
        SImageDataRead* loadImageData();
        
    private:
        
        /* Functions */
        
        void getPNGInfo(s32 ColorType, SImageDataRead* TextureInfo);
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
