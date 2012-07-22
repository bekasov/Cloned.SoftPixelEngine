/*
 * Image loader PCX header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGELOADER_PCX_H__
#define __SP_IMAGELOADER_PCX_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_PCX


#include "FileFormats/Image/spImageFormatInterfaces.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT ImageLoaderPCX : public ImageLoader
{
    
    public:
        
        ImageLoaderPCX(io::File* File);
        ~ImageLoaderPCX();
        
        SImageDataRead* loadImageData();
        
    private:
        
        /* Structures */
        
        struct SHeaderPCX
        {
            u8 manufacturer, version;
            u8 encoding, bpp;
            
            u16 xmin, ymin;
            u16 xmax, ymax;
            u16 horzRes, vertRes;
            
            u8 palette[48];
            u8 reserved, numColorPlanes;
            
            u16 bytesPerScanLine, paletteType;
            u16 horzSize, vertSize;
            
            u8 padding[54];
        };
        
        /* Functions */
        
        void readPCX1bit(SHeaderPCX* pHeader, SImageDataRead* texture);
        void readPCX4bit(SHeaderPCX* pHeader, SImageDataRead* texture);
        void readPCX8bit(SHeaderPCX* pHeader, SImageDataRead* texture);
        void readPCX24bit(SHeaderPCX* pHeader, SImageDataRead* texture);
        
        /* Inline functions */
        
        inline s32 readByte() const
        {
            return static_cast<s32>(File_->readValue<u8>());
        }
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
