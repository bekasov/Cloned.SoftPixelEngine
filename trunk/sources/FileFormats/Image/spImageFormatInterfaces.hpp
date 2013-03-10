/*
 * Image format interfaces header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGEFORMATINTERFACES_H__
#define __SP_IMAGEFORMATINTERFACES_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutput.hpp"
#include "Base/spBaseFileFormatHandler.hpp"
#include "Base/spMath.hpp"
#include "Base/spMaterialColor.hpp"

#include <cstdio>


namespace sp
{
namespace video
{


/*
 * Enumerations
 */

//! Image buffer pixel formats
enum EPixelFormats
{
    PIXELFORMAT_ALPHA = 0,  //!< Alpha channel but no color components.
    PIXELFORMAT_GRAY,       //!< Gray scaled. 1 color component.
    PIXELFORMAT_GRAYALPHA,  //!< Gray scaled with alpha channel. 2 color components.
    PIXELFORMAT_RGB,        //!< Red/ Green/ Blue components in the normal order RGB. 3 color components.
    PIXELFORMAT_BGR,        //!< Red/ Green/ Blue components in the inverse order BGR. 3 color components.
    PIXELFORMAT_RGBA,       //!< Red/ Green/ Blue components with alpha channel in the normal order RGBA. 4 color components.
    PIXELFORMAT_BGRA,       //!< Red/ Green/ Blue components with alpha channel in the inverse order BGRA. 4 color components.
    PIXELFORMAT_DEPTH,      //!< Depth component (floating point).
};

//! Image file formats
enum EImageFileFormats
{
    IMAGEFORMAT_UNKNOWN,    //!< Unknown file format.
    
    /* Supported formats */
    IMAGEFORMAT_BMP,        //!< Windows Bitmap (bmp, dib).
    IMAGEFORMAT_JPG,        //!< Joint Photographic Experts (jpg, jpeg, jpe, jfif).
    IMAGEFORMAT_PNG,        //!< Portable Network Graphics (png).
    IMAGEFORMAT_TGA,        //!< Targa True Image (tga, tpci).
    IMAGEFORMAT_PCX,        //!< Picture Exchange (pcx).
    IMAGEFORMAT_DDS,        //!< Direct Draw Surface (dds).
    IMAGEFORMAT_WAD,        //!< Where is All the Data (wad).
    
    /* Unsupported formats (just for identification) */
    IMAGEFORMAT_GIF,        //!< GraphicsInterchangeFormat (gif) (unsupported, only for identification).
    IMAGEFORMAT_PSD,        //!< Adobe Photoshop (psd) (unsupported, only for identification).
};


/*
 * Structures
 */

struct SImageData
{
    SImageData() :
        Width       (0              ),
        Height      (0              ),
        bpp         (24             ),
        Format      (PIXELFORMAT_RGB),
        FormatSize  (3              )
    {
    }
    virtual ~SImageData()
    {
    }
    
    /* Members */
    s32 Width;
    s32 Height;
    
    s32 bpp;                //!< Bits per pixel (8, 16, 24, 32).
    EPixelFormats Format;
    s32 FormatSize;         //!< Pixel format size. 1, 2, 3 or 4.
};

//! Image data for loading.
struct SImageDataRead : public SImageData
{
    SImageDataRead() :
        SImageData  (   ),
        ID          (0  ),
        ImageBuffer (0  )
    {
    }
    ~SImageDataRead()
    {
        MemoryManager::deleteBuffer(ImageBuffer);
    }
    
    /* Members */
    u32 ID;             // Identification number
    u8* ImageBuffer;    // Image RAW data
};

struct SImageDataWrite : public SImageData
{
    SImageDataWrite() :
        SImageData  (   ),
        ImageBuffer (0  )
    {
    }
    ~SImageDataWrite()
    {
    }
    
    /* Members */
    const u8* ImageBuffer;
};


//! ImageLoader class used as image loading interface.
class SP_EXPORT ImageLoader : public io::BaseFileFormatHandler
{
    
    public:
        
        virtual ~ImageLoader()
        {
        }
        
        /* Functions */
        
        virtual SImageDataRead* loadImageData() = 0;
        
    protected:
        
        ImageLoader(io::File* File) :
            io::BaseFileFormatHandler(File)
        {
        }
        
};

//! ImageSaver class used as image saving interface.
class SP_EXPORT ImageSaver : public io::BaseFileFormatHandler
{
    
    public:
        
        virtual ~ImageSaver()
        {
        }
        
        /* Functions */
        
        virtual bool saveImageData(SImageDataWrite* ImageData) = 0;
        
    protected:
        
        ImageSaver(io::File* File) :
            io::BaseFileFormatHandler(File)
        {
        }
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
