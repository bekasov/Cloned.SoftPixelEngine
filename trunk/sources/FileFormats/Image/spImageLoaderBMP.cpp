/*
 * Image loader BMP file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Image/spImageLoaderBMP.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_BMP


#include "Base/spImageManagement.hpp"


namespace sp
{
namespace video
{


ImageLoaderBMP::ImageLoaderBMP(io::File* File) :
    ImageLoader(File)
{
}
ImageLoaderBMP::~ImageLoaderBMP()
{
}

SImageDataRead* ImageLoaderBMP::loadImageData()
{
    /* Check if the file has opened correct */
    if (!File_ || !File_->hasReadAccess())
        return 0;
    
    /* Allocate new texture RAW data & header buffer */
    SImageDataRead* texture = new SImageDataRead();
    
    SHeaderBMP HeaderInfo;
    
    /* === Header === */
    
    // Read the header
    File_->readBuffer(&HeaderInfo, sizeof(SHeaderBMP));
    
    // Check the identity
    if (HeaderInfo.ID != 0x4D42) // "BM"
    {
        io::Log::error("BMP file has incorrect identity");
        return 0;
    }
    
    // General settings
    bool BottomUpImage = true;
    
    texture->Width  = HeaderInfo.Width;
    
    if (HeaderInfo.Height < 0)
    {
        texture->Height = -HeaderInfo.Height;
        BottomUpImage   = false;
    }
    else
        texture->Height = HeaderInfo.Height;
    
    if (!HeaderInfo.BitmapDataSize)
        HeaderInfo.BitmapDataSize = HeaderInfo.Width * HeaderInfo.Height * HeaderInfo.bpp / 8;
    
    // Compute line pad (rest of each pixel data line is 0 filled)
    const s32 LinePad = (4 - ((texture->Width*3) % 4)) % 4;
    
    /* === Color mask === */
    
    if (HeaderInfo.Compression == BMP_BI_BITFIELDS)
    {
        s32 Bitmasks[3];
        File_->readBuffer(Bitmasks, sizeof(s32), 3);
        
        // No compression support yet
        io::Log::error("BMP compression is not supported yet");
        return 0;
    }
    
    /* === Color pallete === */
    
    u32* Palette    = 0;
    u8* PixelData   = 0;
    
    if (HeaderInfo.Colors)
    {
        Palette = new u32[HeaderInfo.Colors];
        File_->readBuffer(Palette, sizeof(u32), HeaderInfo.Colors);
    }
    else if (HeaderInfo.bpp == 1 || HeaderInfo.bpp == 4 || HeaderInfo.bpp == 8)
    {
        const u32 PaletteSize = (1 << HeaderInfo.bpp);
        Palette = new u32[PaletteSize];
        File_->readBuffer(Palette, sizeof(u32), PaletteSize);
    }
    
    /* === Color data === */
    
    if (HeaderInfo.bpp == 1 || HeaderInfo.bpp == 4 || HeaderInfo.bpp == 8)
    {
        // Allocate memory for the image data
        texture->ImageBuffer    = new u8[HeaderInfo.Width * HeaderInfo.Height * 3];
        PixelData               = new u8[HeaderInfo.BitmapDataSize];
        
        // Read the body
        File_->readBuffer(PixelData, sizeof(u8), HeaderInfo.BitmapDataSize);
    }
    
    // Get the bits per pixel & Allocate the image data memory
    switch (HeaderInfo.bpp)
    {
        case 1:
        {
            for (u32 i = 0, j = 0; j < HeaderInfo.BitmapDataSize; ++j)
            {
                setImagePalettePixel(texture, Palette, i++,  PixelData[j]       & 0x00000001);
                setImagePalettePixel(texture, Palette, i++, (PixelData[j] >> 1) & 0x00000001);
                setImagePalettePixel(texture, Palette, i++, (PixelData[j] >> 2) & 0x00000001);
                setImagePalettePixel(texture, Palette, i++, (PixelData[j] >> 3) & 0x00000001);
                setImagePalettePixel(texture, Palette, i++, (PixelData[j] >> 4) & 0x00000001);
                setImagePalettePixel(texture, Palette, i++, (PixelData[j] >> 5) & 0x00000001);
                setImagePalettePixel(texture, Palette, i++, (PixelData[j] >> 6) & 0x00000001);
                setImagePalettePixel(texture, Palette, i++,  PixelData[j] >> 7              );
            }
        }
        break;
        
        case 4:
        {
            for (u32 i = 0, j = 0; j < HeaderInfo.BitmapDataSize; ++j)
            {
                setImagePalettePixel(texture, Palette, i++, PixelData[j] & 0x00001111);
                setImagePalettePixel(texture, Palette, i++, PixelData[j] >> 4        );
            }
        }
        break;
        
        case 8:
        {
            for (u32 i = 0; i < HeaderInfo.BitmapDataSize; ++i)
                setImagePalettePixel(texture, Palette, i, PixelData[i]);
        }
        break;
        
        case 24:
        {
            // Allocate memory for the image data
            texture->ImageBuffer = new u8[HeaderInfo.BitmapDataSize];
            
            // Read the body
            if (LinePad)
            {
                for (s32 y = 0, i = 0; y < texture->Height; ++y, i += texture->Width*3)
                {
                    File_->readBuffer(texture->ImageBuffer + i, sizeof(u8), texture->Width*3);
                    File_->setSeek(LinePad, io::FILEPOS_CURRENT);
                }
            }
            else
                File_->readBuffer(texture->ImageBuffer, sizeof(u8), HeaderInfo.BitmapDataSize);
        }
        break;
        
        default:
        {
            io::Log::error("BMP file has unsupported bpp value (" + io::stringc(HeaderInfo.bpp) + ")");
            
            // Delete the pallete
            MemoryManager::deleteMemory(Palette);
            MemoryManager::deleteBuffer(PixelData);
        }
        return 0;
    }
    
    // Delete the pallete
    MemoryManager::deleteBuffer(Palette);
    MemoryManager::deleteBuffer(PixelData);
    
    // Flip colors
    if (BottomUpImage)
        ImageConverter::flipImageColors(texture->ImageBuffer, texture->Width, texture->Height, 3);
    
    // Flip Y axis
    ImageConverter::flipImageVert(texture->ImageBuffer, texture->Width, texture->Height, 3);
    
    return texture;
}

void ImageLoaderBMP::setImagePalettePixel(
    SImageDataRead* Texture, const u32* Palette, const u32 ImageOffset, const u32 PaletteOffset)
{
    Texture->ImageBuffer[ImageOffset*3+0] = video::getBlue  (Palette[PaletteOffset]);
    Texture->ImageBuffer[ImageOffset*3+1] = video::getGreen (Palette[PaletteOffset]);
    Texture->ImageBuffer[ImageOffset*3+2] = video::getRed   (Palette[PaletteOffset]);
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
