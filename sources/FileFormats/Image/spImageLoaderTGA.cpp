/*
 * Image loader TGA file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Image/spImageLoaderTGA.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_TGA


#include "Base/spImageManagement.hpp"


namespace sp
{
namespace video
{


ImageLoaderTGA::ImageLoaderTGA(io::File* File) :
    ImageLoader(File)
{
}
ImageLoaderTGA::~ImageLoaderTGA()
{
}

SImageDataRead* ImageLoaderTGA::loadImageData()
{
    /* Check if the file has opened correct */
    if (!File_ || !File_->hasReadAccess())
        return 0;
    
    /* Allocate new texture RAW data & header buffer */
    SImageDataRead* texture = new SImageDataRead();
    
    /* Read the header */
    
    if (File_->readBuffer(&MainHeader_, sizeof(SHeaderTGA)) <= 0)
    {
        io::Log::error("Could not read TGA header");
        MemoryManager::deleteMemory(texture);
        return 0;
    }
    
    if (MainHeader_.ImageType == TGA_IMGDATA_RGB)
    {
        if (!loadUncompressedTGA(texture))
            return 0;
    }
    else if (MainHeader_.ImageType == TGA_IMGDATA_RGB_COMPRESSED)
    {
        if (!loadCompressedTGA(texture))
            return 0;
    }
    else
    {
        io::Log::error("TGA header type must be 2 or 10");
        MemoryManager::deleteMemory(texture);
        return 0;
    }
    
    // Flip image vertical if the origin is at the bottom
    if (!(MainHeader_.ImageDescriptor & 0x20))
        ImageConverter::flipImageVert(texture->ImageBuffer, texture->Width, texture->Height, texture->FormatSize);
    
    return texture;
}


/*
 * ======= Private: =======
 */

bool ImageLoaderTGA::loadUncompressedTGA(SImageDataRead* texture)
{
    texture->Format     = PIXELFORMAT_RGBA;     // Standard format
    texture->FormatSize = 4;                    // Standard internal format
    
    texture->Width  = MainHeader_.ImageWidth;
    texture->Height = MainHeader_.ImageHeight;
    texture->bpp    = MainHeader_.bpp;          // Grab The TGA's Bits Per Pixel (24 or 32)
    
    if ( texture->Width <= 0 || texture->Height <= 0 || ( texture->bpp != 24 && texture->bpp != 32 ) )
    {
        io::Log::error("Invalid TGA header information (size/ bpp)");
        MemoryManager::deleteMemory(texture);
        return false;
    }
    
    const u32 BytesPerPixel     = texture->bpp/8;                                   // Divide By 8 To Get The Bytes Per Pixel
    const u32 ImageBufferSize   = texture->Width * texture->Height * BytesPerPixel; // Calculate The Memory Required For The TGA Data
    
    texture->ImageBuffer = new u8[ImageBufferSize];                                 // Reserve Memory To Hold The TGA Data
    
    if (!texture->ImageBuffer || File_->readBuffer(texture->ImageBuffer, ImageBufferSize) != ImageBufferSize)
    {
        io::Log::error("Incorrect TGA image data size");
        
        MemoryManager::deleteBuffer(texture->ImageBuffer);
        MemoryManager::deleteMemory(texture);
        
        return false;
    }
    
    // Flip colors
    ImageConverter::flipImageColors(texture->ImageBuffer, texture->Width, texture->Height, BytesPerPixel);
    
    if (texture->bpp == 24)
    {
        texture->Format     = PIXELFORMAT_RGB;
        texture->FormatSize = 3;
    }
    
    return true;
}

bool ImageLoaderTGA::loadCompressedTGA(SImageDataRead* texture)
{
    texture->Format     = PIXELFORMAT_RGBA;     // Standard format
    texture->FormatSize = 4;                    // Standard internal format
    
    texture->Width  = MainHeader_.ImageWidth;
    texture->Height = MainHeader_.ImageHeight;
    texture->bpp    = MainHeader_.bpp;          // Grab The TGA's Bits Per Pixel (24 or 32)
    
    if ( texture->Width <= 0 || texture->Height <= 0 || ( texture->bpp != 24 && texture->bpp != 32 ) )
    {
        io::Log::error("Invalid TGA file information (size/ bpp)");
        MemoryManager::deleteMemory(texture);
        return false;
    }
    
    const u32 BytesPerPixel     = texture->bpp/8;                                   // Divide By 8 To Get The Bytes Per Pixel
    const u32 ImageBufferSize   = texture->Width * texture->Height * BytesPerPixel; // Calculate The Memory Required For The TGA Data
    
    texture->ImageBuffer        = new u8[ImageBufferSize];                          // Reserve Memory To Hold The TGA Data
    
    // Decompressing operations
    
    u32 PixelCount      = texture->Width * texture->Height; // Number of pixels in the image
    u32 CurrentPixel    = 0;                                // Current pixel being read
    u32 CurrentByte     = 0;                                // Current byte 
    u8* ColorBuffer     = new u8[BytesPerPixel];            // Storage for 1 pixel
    
    do
    {
        
        u8 ChunkHeader = 0;
        
        if (File_->readBuffer(&ChunkHeader, sizeof(u8)) <= 0)
        {
            io::Log::error("Could not read TGA RLE header");
            
            MemoryManager::deleteBuffer(texture->ImageBuffer);
            MemoryManager::deleteMemory(texture);
            
            return false;
        }
        
        if (ChunkHeader < 128)
        {
            
            ++ChunkHeader;
            
            for (s16 Counter = 0; Counter < ChunkHeader; ++Counter)
            {
                
                if (File_->readBuffer(ColorBuffer, BytesPerPixel) != BytesPerPixel)
                {
                    io::Log::error("Could not read TGA file image data");
                    
                    MemoryManager::deleteBuffer(ColorBuffer);
                    MemoryManager::deleteBuffer(texture->ImageBuffer);
                    MemoryManager::deleteMemory(texture);
                    
                    return false;
                }
                
                texture->ImageBuffer[CurrentByte + 0] = ColorBuffer[2];
                texture->ImageBuffer[CurrentByte + 1] = ColorBuffer[1];
                texture->ImageBuffer[CurrentByte + 2] = ColorBuffer[0];
                
                if (BytesPerPixel == 4)
                    texture->ImageBuffer[CurrentByte + 3] = ColorBuffer[3];
                
                CurrentByte += BytesPerPixel;
                ++CurrentPixel;
                
                if (CurrentPixel > PixelCount)
                {
                    io::Log::error("TGA file has to many pixels");
                    
                    MemoryManager::deleteBuffer(ColorBuffer);
                    MemoryManager::deleteBuffer(texture->ImageBuffer);
                    MemoryManager::deleteMemory(texture);
                    
                    return false;
                }
                
            }
            
        }
        else
        {
            
            ChunkHeader -= 127;
            
            if (File_->readBuffer(ColorBuffer, BytesPerPixel) != BytesPerPixel)
            {
                io::Log::error("TGA file is unreadable");
                
                MemoryManager::deleteBuffer(ColorBuffer);
                MemoryManager::deleteBuffer(texture->ImageBuffer);
                MemoryManager::deleteMemory(texture);
                
                return false;
            }
            
            for (s16 Counter = 0; Counter < ChunkHeader; ++Counter)
            {
                texture->ImageBuffer[CurrentByte + 0] = ColorBuffer[2];
                texture->ImageBuffer[CurrentByte + 1] = ColorBuffer[1];
                texture->ImageBuffer[CurrentByte + 2] = ColorBuffer[0];
                
                if (BytesPerPixel == 4)
                    texture->ImageBuffer[CurrentByte + 3] = ColorBuffer[3];
                
                CurrentByte += BytesPerPixel;
                ++CurrentPixel;
                
                if (CurrentPixel > PixelCount)
                {
                    io::Log::error("TGA file has to many pixels");
                    
                    MemoryManager::deleteBuffer(ColorBuffer);
                    MemoryManager::deleteBuffer(texture->ImageBuffer);
                    MemoryManager::deleteMemory(texture);
                    
                    return false;
                }
            }
            
        }
        
    }
    while (CurrentPixel < PixelCount);
    
    if (texture->bpp == 24)
    {
        texture->Format     = PIXELFORMAT_RGB;
        texture->FormatSize = 3;
    }
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
