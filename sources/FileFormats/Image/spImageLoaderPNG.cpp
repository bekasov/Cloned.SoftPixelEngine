/*
 * Image loading PNG file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Image/spImageLoaderPNG.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_PNG


#include "Base/spImageManagement.hpp"

//#if defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)
#   include "Plugins/libpng/png.h"
/*#else
#   include <png.h> // "libpng.a"
#endif*/


namespace sp
{
namespace video
{


ImageLoaderPNG::ImageLoaderPNG(io::File* File) :
    ImageLoader(File)
{
}
ImageLoaderPNG::~ImageLoaderPNG()
{
}

void spPNGReadCallback(png_struct* png_ptr, png_byte* data, png_size_t length)
{
    io::File* FileStream = (io::File*)png_ptr->io_ptr;
    FileStream->readBuffer(data, length);
}

SImageDataRead* ImageLoaderPNG::loadImageData()
{
    /* Check if the file was opened correctly */
    if (!File_ || !File_->hasReadAccess())
        return 0;
    
    png_byte    magic[8];
    png_structp png_ptr;
    png_infop   info_ptr;
    
    s32 bit_depth, color_type;
    png_bytep* row_pointers = 0;
    
    // Read magic number
    File_->readBuffer(magic, sizeof(png_byte), 8);
    
    // Check for valid magic number
    if (!png_check_sig(magic, sizeof(magic)))
    {
        io::Log::error("PNG file has incorrect magic number");
        return 0;
    }
    
    // Create a png read struct
    if ( !( png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0) ) )
        return 0;
    
    // Create a png info struct
    if ( !( info_ptr = png_create_info_struct(png_ptr) ) )
    {
        png_destroy_read_struct(&png_ptr, 0, 0);
        return 0;
    }
    
    /* Allocate new texture RAW data & header buffer */
    SImageDataRead* texture = new SImageDataRead();
    
    // Initialize the setjmp for returning properly after a libpng error occured
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        
        MemoryManager::deleteBuffer(row_pointers);
        
        if (texture)
        {
            MemoryManager::deleteBuffer(texture->ImageBuffer);
            MemoryManager::deleteMemory(texture);
        }
        
        return 0;
    }
    
    // Setup libpng for using standard C fread() function with our FILE pointer
    png_set_read_fn(png_ptr, File_, spPNGReadCallback);
    
    // tell libpng that we have already read the magic number
    png_set_sig_bytes(png_ptr, sizeof(magic));
    
    // Read png info
    png_read_info(png_ptr, info_ptr);
    
    // Get some usefull information from header
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    
    // Convert index color images to RGB images
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    
    // Convert 1-2-4 bits grayscale images to 8 bits grayscale
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_gray_1_2_4_to_8(png_ptr);
    
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);
    
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    else if (bit_depth < 8)
        png_set_packing(png_ptr);
    
    // Update info structure to apply transformations
    png_read_update_info(png_ptr, info_ptr);
    
    // Retrieve updated information
    png_get_IHDR(
        png_ptr, info_ptr,
        (png_uint_32*)(&texture->Width),
        (png_uint_32*)(&texture->Height),
		&bit_depth, &color_type,
		0, 0, 0
    );
    
    // Get image format and components per pixel
    getPNGInfo(color_type, texture);
    
    // We can now allocate memory for storing pixel data
    texture->ImageBuffer = new u8[ texture->Width * texture->Height * texture->FormatSize ];
    
    // Setup a pointer array. Each one points at the begening of a row
    row_pointers = new png_bytep[texture->Height];
    
    for (s32 i = 0; i < texture->Height; ++i)
    {
        row_pointers[i] =
            (png_bytep)(texture->ImageBuffer +
	        ((texture->Height - (i + 1)) * texture->Width * texture->FormatSize));
    }
    
    // Read pixel data using row pointers
    png_read_image(png_ptr, row_pointers);
    
    // Finish decompression and release memory
    png_read_end(png_ptr, 0);
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    
    // We don't need row pointers anymore
    MemoryManager::deleteBuffer(row_pointers);
    
    // Flip image data y axis
    ImageConverter::flipImageVert(texture->ImageBuffer, texture->Width, texture->Height, texture->FormatSize);
    
    return texture;
}


/*
 * ======= Private: =======
 */

void ImageLoaderPNG::getPNGInfo(s32 ColorType, SImageDataRead* TextureInfo)
{
    switch (ColorType)
    {
        case PNG_COLOR_TYPE_GRAY:
            TextureInfo->Format     = PIXELFORMAT_GRAY;
            TextureInfo->FormatSize = 1;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            TextureInfo->Format     = PIXELFORMAT_GRAYALPHA;
            TextureInfo->FormatSize = 2;
            break;
        case PNG_COLOR_TYPE_RGB:
            TextureInfo->Format     = PIXELFORMAT_RGB;
            TextureInfo->FormatSize = 3;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            TextureInfo->Format     = PIXELFORMAT_RGBA;
            TextureInfo->FormatSize = 4;
            break;
        default:
            io::Log::error("Invalid color type in PNG file");
            break;
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
