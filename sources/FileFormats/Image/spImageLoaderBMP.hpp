/*
 * Image loader BMP header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGELOADER_BMP_H__
#define __SP_IMAGELOADER_BMP_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_BMP


#include "FileFormats/Image/spImageFormatInterfaces.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT ImageLoaderBMP : public ImageLoader
{
    
    public:
        
        ImageLoaderBMP(io::File* File);
        ~ImageLoaderBMP();
        
        SImageDataRead* loadImageData();
        
    private:
        
        /* Macros */
        
        static const s32 BMP_BI_RGB         = 0;
        static const s32 BMP_BI_RLE8        = 1;
        static const s32 BMP_BI_RLE4        = 2;
        static const s32 BMP_BI_BITFIELDS   = 3;
        
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
            u32	FileSize, Reserved;
            u32	BitmapDataOffset, BitmapHeaderSize;
            s32 Width, Height;
            u16 Planes, bpp;
            u32 Compression, BitmapDataSize;
            s32 PixelPerMeterX, PixelPerMeterY;
            u32 Colors, ImportantColors;
        }
        SP_PACK_STRUCT;
        
        #ifdef _MSC_VER
        #   pragma pack(pop, packing)
        #endif
        
        #undef SP_PACK_STRUCT
        
        /* Functions */
        
        void setImagePalettePixel(
            SImageDataRead* Texture, const u32* Palette, const u32 ImageOffset, const u32 PaletteOffset
        );
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
