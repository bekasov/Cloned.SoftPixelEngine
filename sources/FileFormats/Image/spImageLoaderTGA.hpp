/*
 * Image loader TGA header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGELOADER_TGA_H__
#define __SP_IMAGELOADER_TGA_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_TGA


#include "FileFormats/Image/spImageFormatInterfaces.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT ImageLoaderTGA : public ImageLoader
{
    
    public:
        
        ImageLoaderTGA(io::File* File);
        ~ImageLoaderTGA();
        
        SImageDataRead* loadImageData();
        
    private:
        
        /* Enumerations */
        
        enum ETGAImageTypes
        {
            TGA_IMGDATA_NONE                    = 0,
            
            TGA_IMGDATA_INDEXED                 = 1,
            TGA_IMGDATA_RGB                     = 2,
            TGA_IMGDATA_MONOCHROM               = 3,
            
            TGA_IMGDATA_INDEXED_COMPRESSED      = 9,
            TGA_IMGDATA_RGB_COMPRESSED          = 10,
            TGA_IMGDATA_MONOCHROM_COMPRESSED    = 11,
        };
        
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
        
        struct SHeaderTGA
        {
            u8 IDSize;
            u8 ColorMapType;
            u8 ImageType; // (ETGAImageTypes)
            
            u16 ColorMapStart;
            u16 ColorMapSize;
            u8 ColorMapBits;
            
            u16 OriginX;
            u16 OriginY;
            u16 ImageWidth;
            u16 ImageHeight;
            
            u8 bpp;
            u8 ImageDescriptor;
        }
        SP_PACK_STRUCT;
        
        #ifdef _MSC_VER
        #	pragma pack(pop, packing)
        #endif
        
        #undef SP_PACK_STRUCT
        
        /* Functions */
        
        bool loadUncompressedTGA(SImageDataRead* texture);
        bool loadCompressedTGA(SImageDataRead* texture);
        
        /* Members */
        
        SHeaderTGA MainHeader_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
