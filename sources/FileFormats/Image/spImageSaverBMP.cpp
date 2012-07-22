/*
 * Image saver BMP file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Image/spImageSaverBMP.hpp"


namespace sp
{
namespace video
{


ImageSaverBMP::ImageSaverBMP(io::File* File) :
    ImageSaver(File)
{
}
ImageSaverBMP::~ImageSaverBMP()
{
}

bool ImageSaverBMP::saveImageData(SImageDataWrite* ImageData)
{
    /* Check if the file has opened correct */
    if (!File_ || !File_->hasWriteAccess())
        return false;
    
    /* Get the image data */
    const u8* ImageBuffer = ImageData->ImageBuffer;
    
    /* Get the internal format */
    const s32 FormatSize = ImageData->FormatSize;
    
    /* Check if the internal format is big anough */
    if (FormatSize < 3)
    {
        io::Log::error(
            "Format size " + io::stringc(FormatSize) + " for BMP image file is not supported"
        );
        return false;
    }
    
    /* Header settings */
    SHeaderBMP HeaderInfo;
    HeaderInfo.ID               = 0x4D42; // ("BM")
    HeaderInfo.Reserved         = 0;
    HeaderInfo.BitmapDataOffset = sizeof(HeaderInfo);
    HeaderInfo.BitmapHeaderSize = 0x28;
    HeaderInfo.Width            = ImageData->Width;
    HeaderInfo.Height           = ImageData->Height;
    HeaderInfo.Planes           = 1;
    HeaderInfo.bpp              = 24;
    HeaderInfo.Compression      = 0;
    HeaderInfo.PixelPerMeterX   = 0;
    HeaderInfo.PixelPerMeterY   = 0;
    HeaderInfo.Colors           = 0;
    HeaderInfo.ImportantColors  = 0;
    HeaderInfo.BitmapDataSize   = HeaderInfo.Width * HeaderInfo.Height * HeaderInfo.bpp / 8;
    HeaderInfo.FileSize         = HeaderInfo.BitmapDataOffset + HeaderInfo.BitmapDataSize;
    
    /* Write header into the image file */
    File_->writeBuffer(&HeaderInfo, sizeof(HeaderInfo));
    
    /* Loop for the image data */
    for (s32 y = HeaderInfo.Height - 1, x; y >= 0; --y)
    {
        for (x = 0; x < HeaderInfo.Width; ++x)
        {
            /* Write the image data */
            File_->writeValue<u8>(ImageBuffer[ (y * HeaderInfo.Width + x)*FormatSize + 2 ]);
            File_->writeValue<u8>(ImageBuffer[ (y * HeaderInfo.Width + x)*FormatSize + 1 ]);
            File_->writeValue<u8>(ImageBuffer[ (y * HeaderInfo.Width + x)*FormatSize + 0 ]);
        }
    }
    
    /* Loading successful */
    return true;
}


} // /namespace video

} // /namespace sp



// ================================================================================
