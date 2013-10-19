/*
 * Image loader JPG file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Image/spImageLoaderJPG.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_JPG


#include "Plugins/jpeglib/jpeglib.h"

//#include <jpeglib.h>
#include <setjmp.h>
#include <boost/shared_array.hpp>


namespace sp
{
namespace video
{


ImageLoaderJPG::ImageLoaderJPG(io::File* File) :
    ImageLoader(File)
{
}
ImageLoaderJPG::~ImageLoaderJPG()
{
}

SImageDataRead* ImageLoaderJPG::loadImageData()
{
    /* Check if the file has opened correct */
    if (!File_ || !File_->hasReadAccess())
        return 0;
    
    /* Allocate new texture RAW data & header buffer */
    SImageDataRead* texture = new SImageDataRead();
    
    jpeg_decompress_struct cInfo;
    jpeg_error_mgr jerr;
    
    cInfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cInfo);
    
    /* Read complete file into a buffer */
    const u32 FileSize = File_->getSize();
    boost::shared_array<u8> FileBuffer(new u8[FileSize]);
    File_->readBuffer(FileBuffer.get(), FileSize);
    
    jpeg_mem_src(&cInfo, FileBuffer.get(), FileSize);
    
    /* Decompress the JPG image data */
    if (!decompressJPG(&cInfo, texture))
    {
        delete texture;
        return 0;
    }
    
    jpeg_destroy_decompress(&cInfo);
    
    return texture;
}


/*
 * ======= Private: =======
 */

bool ImageLoaderJPG::decompressJPG(jpeg_decompress_struct* cInfo, SImageDataRead* TextureInfo)
{
    /* Start the decompression */
    if (jpeg_read_header(cInfo, true) == JPEG_SUSPENDED)
    {
        io::Log::error("Reading JPEG header failed");
        return false;
    }
    
    jpeg_start_decompress(cInfo);
    
    /* Settings */
    s32 RowSpan                 = cInfo->image_width * cInfo->num_components;
    TextureInfo->Width          = cInfo->image_width;
    TextureInfo->Height         = cInfo->image_height;
    TextureInfo->ImageBuffer    = new u8[RowSpan * TextureInfo->Height];
    
    /* Loop the array y-axis */
    std::vector<u8*> rowPtr(TextureInfo->Height);

    for (s32 i = 0; i < TextureInfo->Height; ++i)
        rowPtr[i] = &(TextureInfo->ImageBuffer[i*RowSpan]);
    
    u32 rowsRead = 0;
    while (cInfo->output_scanline < cInfo->output_height)
        rowsRead += jpeg_read_scanlines(cInfo, &rowPtr[rowsRead], cInfo->output_height - rowsRead);
    
    /* Finish the decompression */
    jpeg_finish_decompress(cInfo);
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
