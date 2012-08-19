/*
 * Image saver BMP header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGESAVER_BMP_H__
#define __SP_IMAGESAVER_BMP_H__


#include "Base/spStandard.hpp"
#include "Base/spImageManagement.hpp"
#include "FileFormats/Image/spImageLoaderBMP.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT ImageSaverBMP : public ImageSaver
{
    
    public:
        
        ImageSaverBMP(io::File* File);
        ~ImageSaverBMP();
        
        bool saveImageData(SImageDataWrite* ImageData);
        
    private:
        
        /* Structures */
        
        #if defined(_MSC_VER)
        #   pragma pack(push, packing)
        #   pragma pack(1)
        #   define SP_PACK_STRUCT
        #elif defined(__GNUC__)
        #   define SP_PACK_STRUCT __attribute__((packed))
        #else
        #   define SP_PACK_STRUCT
        #endif
        
        struct SHeaderBMP
        {
            u16 ID;
            u32    FileSize, Reserved;
            u32    BitmapDataOffset, BitmapHeaderSize;
            u32 Width, Height;
            u16 Planes, bpp;
            u32 Compression, BitmapDataSize;
            u32 PixelPerMeterX, PixelPerMeterY;
            u32 Colors, ImportantColors;
        }
        SP_PACK_STRUCT;
        
        #ifdef _MSC_VER
        #   pragma pack(pop, packing)
        #endif
        
        #undef SP_PACK_STRUCT
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
