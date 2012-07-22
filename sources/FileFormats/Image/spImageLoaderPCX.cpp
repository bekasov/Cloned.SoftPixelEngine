/*
 * Image loader PCX file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Image/spImageLoaderPCX.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_PCX


#include "Base/spImageManagement.hpp"


namespace sp
{
namespace video
{


ImageLoaderPCX::ImageLoaderPCX(io::File* File) :
    ImageLoader(File)
{
}
ImageLoaderPCX::~ImageLoaderPCX()
{
}

SImageDataRead* ImageLoaderPCX::loadImageData()
{
    /* Check if the file has opened correct */
    if (!File_ || !File_->hasReadAccess())
        return 0;
    
    SHeaderPCX HeaderInfo;
    
    // Checking for header version
    File_->readBuffer(&HeaderInfo, sizeof(SHeaderPCX));
    
    if (HeaderInfo.manufacturer != 0x0A)
    {
        io::Log::error("PCX file has an incorrect version number: " + io::stringc(HeaderInfo.manufacturer));
        return 0;
    }
    
    /* Allocate new texture RAW data & header buffer */
    SImageDataRead* texture = new SImageDataRead();
    
    // Initialize texture parameters
    texture->Width          = HeaderInfo.xmax - HeaderInfo.xmin + 1;
    texture->Height         = HeaderInfo.ymax - HeaderInfo.ymin + 1;
    texture->Format         = PIXELFORMAT_RGB;
    texture->FormatSize     = 3;
    texture->ImageBuffer    = new u8[ texture->Width * texture->Height * texture->FormatSize ];
    
    switch (HeaderInfo.bpp)
    {
        case 1:
            readPCX1bit(&HeaderInfo, texture); break;
        case 4:
            readPCX4bit(&HeaderInfo, texture); break;
        case 8:
            readPCX8bit(&HeaderInfo, texture); break;
        case 24:
            readPCX24bit(&HeaderInfo, texture); break;
        default:
            io::Log::error("PCX file has unknown bitcount: " + io::stringc(HeaderInfo.bpp));
            MemoryManager::deleteBuffer(texture->ImageBuffer);
            MemoryManager::deleteMemory(texture);
            return 0;
    }
    
    if (!texture->ImageBuffer)
        return 0;
    
    // Flip Y axis
    ImageConverter::flipImageVert(texture->ImageBuffer, texture->Width, texture->Height, 3);
    
    return texture;
}


/*
 * ======= Private: =======
 */

void ImageLoaderPCX::readPCX1bit(SHeaderPCX* pHeader, SImageDataRead* texture)
{
    s32 y, i, bytes;
    s32 colorIndex;
    s32 rle_count = 0, rle_value = 0;
    u8* ptr = texture->ImageBuffer;
    
    for (y = 0; y < texture->Height; ++y)
    {
        ptr = &texture->ImageBuffer[(texture->Height - (y + 1)) * texture->Width * 3];
        bytes = pHeader->bytesPerScanLine;
        
        // Decode line number y
        while (bytes--)
        {
            if (rle_count == 0)
            {
                if ((rle_value = readByte()) < 0xC0)
                    rle_count = 1;
                else
                {
                    rle_count = rle_value - 0xC0;
                    rle_value = readByte();
                }
            }
            
            --rle_count;
            
            for (i = 7; i >= 0; --i, ptr += 3)
            {
                colorIndex = ((rle_value & (1 << i)) > 0);
                
                ptr[0] = pHeader->palette[colorIndex * 3 + 0];
                ptr[1] = pHeader->palette[colorIndex * 3 + 1];
                ptr[2] = pHeader->palette[colorIndex * 3 + 2];
            }
        }
    }
}

void ImageLoaderPCX::readPCX4bit(SHeaderPCX* pHeader, SImageDataRead* texture)
{
    u8* colorIndex, * line;
    u8* pLine, * ptr;
    s32 rle_count = 0, rle_value = 0;
    s32 x, y, c;
    s32 bytes;
    
    colorIndex = new u8[texture->Width];
    line = new u8[pHeader->bytesPerScanLine];
    
    for (y = 0; y < texture->Height; ++y)
    {
        ptr = &texture->ImageBuffer[(texture->Height - (y + 1)) * texture->Width * 3];
        
        memset(colorIndex, 0, texture->Width * sizeof(u8));
        
        for (c = 0; c < 4; ++c)
        {
            pLine = line;
            bytes = pHeader->bytesPerScanLine;
            
            // Decode line number y
            while (bytes--)
            {
                if (rle_count == 0)
                {
                    if ((rle_value = readByte()) < 0xc0)
                        rle_count = 1;
                    else
                    {
                        rle_count = rle_value - 0xc0;
                        rle_value = readByte();
                    }
                }
                
                --rle_count;
                *(pLine++) = rle_value;
            }
            
            // Compute line's color indexes
            for (x = 0; x < texture->Width; ++x)
            {
                if (line[x / 8] & (128 >> (x % 8)))
                    colorIndex[x] += (1 << c);
            }
        }
        
        // Decode scan line. color index => rgb
        for (x = 0; x < texture->Width; ++x, ptr += 3)
        {
            ptr[0] = pHeader->palette[colorIndex[x] * 3 + 0];
            ptr[1] = pHeader->palette[colorIndex[x] * 3 + 1];
            ptr[2] = pHeader->palette[colorIndex[x] * 3 + 2];
        }
    }
    
    delete [] colorIndex;
    delete [] line;
}

void ImageLoaderPCX::readPCX8bit(SHeaderPCX* pHeader, SImageDataRead* texture)
{
    s32 rle_count = 0, rle_value = 0;
    u8 palette[768];
    u8 magic;
    u8* ptr;
    s32 y, bytes;
    
    // The palette is contained in the last 769 bytes of the file
    s32 curpos = File_->getSeek();
    File_->setSeek(-769, io::FILEPOS_END);
    magic = readByte();
    
    // First byte must be equal to 0x0c (12)
    if (magic != 0x0C)
    {
        io::Log::error(io::stringc("PCX file's colormap's first byte must be 0x0C"));
        MemoryManager::deleteBuffer(texture->ImageBuffer);
        return;
    }
    
    // Read palette
    File_->readBuffer(palette, sizeof(u8), 768);
    File_->setSeek(curpos, io::FILEPOS_BEGIN);
    
    // Read pixel data
    for (y = 0; y < texture->Height; ++y)
    {
        ptr = &texture->ImageBuffer[(texture->Height - (y + 1)) * texture->Width * 3];
        bytes = pHeader->bytesPerScanLine;
        
        // Decode line number y
        while (bytes--)
        {
            if (rle_count == 0)
            {
                if ((rle_value = readByte()) < 0xC0)
                    rle_count = 1;
                else
                {
                    rle_count = rle_value - 0xc0;
                    rle_value = readByte();
                }
            }
            
            --rle_count;
            
            ptr[0] = palette[rle_value * 3 + 0];
            ptr[1] = palette[rle_value * 3 + 1];
            ptr[2] = palette[rle_value * 3 + 2];
            ptr += 3;
        }
    }
}

void ImageLoaderPCX::readPCX24bit(SHeaderPCX* pHeader, SImageDataRead* texture)
{
    u8* ptr = texture->ImageBuffer;
    s32 rle_count = 0, rle_value = 0;
    s32 y, c;
    s32 bytes;
    
    for (y = 0; y < texture->Height; ++y)
    {
        for (c = 0; c < 3; ++c)
        {
            ptr = &texture->ImageBuffer[(texture->Height - (y + 1)) * texture->Width * 3];
            bytes = pHeader->bytesPerScanLine;
            
            // Decode line number y
            while (bytes--)
            {
                if (rle_count == 0)
                {
                    if ((rle_value = readByte()) < 0xc0)
                        rle_count = 1;
                    else
                    {
                        rle_count = rle_value - 0xc0;
                        rle_value = readByte();
                    }
                }
                
                --rle_count;
                ptr[c] = (u8)rle_value;
                ptr += 3;
            }
        }
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
