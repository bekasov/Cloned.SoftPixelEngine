/*
 * Image loader DDS file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Image/spImageLoaderDDS.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_DDS


#include "Base/spImageManagement.hpp"


namespace sp
{
namespace video
{


ImageLoaderDDS::ImageLoaderDDS(io::File* File) :
    ImageLoader     (File   ),
    TexData_        (0      ),
    ImageBufferSize_(0      )
{
}
ImageLoaderDDS::~ImageLoaderDDS()
{
}

SImageDataRead* ImageLoaderDDS::loadImageData()
{
    /* Check if the file has opened correct */
    if (!File_ || !File_->hasReadAccess())
        return 0;
    
    /* Read the header */
    if (!readHeader())
        return 0;
    
    /* Read the body */
    if (!readBody())
        MemoryManager::deleteMemory(TexData_);
    
    return TexData_;
}


/*
 * ======= Private: =======
 */

bool ImageLoaderDDS::readHeader()
{
    /* Read magic number */
    if (File_->readValue<s32>() != 0x20534444)
    {
        io::Log::error("Wrong magic number in DDS file");
        return false;
    }
    
    /* Allocate a new raw texture data structure */
    TexData_ = new SImageDataRead();
    
    /* Read the header */
    File_->readBuffer(&MainHeader_, sizeof(SHeaderDDS));
    
    /* Examine the header information */
    isMipMapped_    = (MainHeader_.Flags & DDSFLAG_MIPMAPS          ) != 0;
    isDepth_        = (MainHeader_.Flags & DDSFLAG_DEPTH            ) != 0;
    
    isCompressed_   = (MainHeader_.Format.Flags & DDSFLAG_COMPRESSED) != 0;
    isAlpha_        = (MainHeader_.Format.Flags & DDSFLAG_ALPHA     ) != 0;
    
    isCubeMap_      = (MainHeader_.CubeMapFlags & DDSFLAG_CUBEMAP   ) != 0;
    
    if (!updateFourCCType())
        return false;
    
    isHeaderDX10_ = false;
    
    if (FourCC_ == FOURCC_DX10)
    {
        File_->readBuffer(&MainHeaderEx_, sizeof(SHeaderDX10DDS));
        isHeaderDX10_ = true;
    }
    
    /* Initialize raw texture data */
    if (isCubeMap_)
        MainHeader_.Height *= 6;
    
    if (isDepth_ && MainHeader_.Depth > 0)
        MainHeader_.Height *= MainHeader_.Depth;
    
    TexData_->Width         = MainHeader_.Width;
    TexData_->Height        = MainHeader_.Height;
    
    if (isAlpha_)
    {
        if (isCompressed_ || MainHeader_.Format.RGBBitCount == 32)
        {
            TexData_->FormatSize    = 4;
            TexData_->Format        = PIXELFORMAT_RGBA;
        }
        else if (MainHeader_.Format.RGBBitCount == 16)
        {
            TexData_->FormatSize    = 2;
            TexData_->Format        = PIXELFORMAT_GRAYALPHA;
        }
    }
    else
    {
        if (isCompressed_ || MainHeader_.Format.RGBBitCount == 24)
        {
            TexData_->FormatSize    = 3;
            TexData_->Format        = PIXELFORMAT_RGB;
        }
        else if (MainHeader_.Format.RGBBitCount == 8)
        {
            TexData_->FormatSize    = 1;
            TexData_->Format        = PIXELFORMAT_GRAY;
        }
    }
    
    /* Allocate image buffer */
    ImageBufferSize_ = MainHeader_.Width * MainHeader_.Height;
    
    ImageBufferSize_ *= (isAlpha_ ? 4 : 3);
    
    TexData_->ImageBuffer = new u8[ImageBufferSize_];
    
    #if 0 // !!!
    //io::getMessage(FourCCName_);
    io::getMessage((s32)MainHeader_.Format.FourCC);
    #endif
    
    return true;
}

bool ImageLoaderDDS::updateFourCCType()
{
    const s32 FourCC = MainHeader_.Format.FourCC;
    
    FourCCName_ = io::stringc::space(4);
    memcpy(&FourCCName_[0], &FourCC, 4);
    
    if (FourCCName_ == "DXT1")
        FourCC_ = FOURCC_DXT1;
    else if (FourCCName_ == "DXT2")
        FourCC_ = FOURCC_DXT2, isAlpha_ = true;
    else if (FourCCName_ == "DXT3")
        FourCC_ = FOURCC_DXT3, isAlpha_ = true;
    else if (FourCCName_ == "DXT4")
        FourCC_ = FOURCC_DXT4;
    else if (FourCCName_ == "DXT5")
        FourCC_ = FOURCC_DXT5;
    else if (FourCCName_ == "DX10")
        FourCC_ = FOURCC_DX10;
    else if (FourCCName_ == "BC4U")
        FourCC_ = FOURCC_BC4U;
    else if (FourCCName_ == "BC4S")
        FourCC_ = FOURCC_BC4S;
    else if (FourCCName_ == "BC5S")
        FourCC_ = FOURCC_BC5S;
    else if (FourCCName_ == "ATI2")
        FourCC_ = FOURCC_ATI2;
    else if (FourCCName_ == "RGBG")
        FourCC_ = FOURCC_RGBG;
    else if (FourCCName_ == "GRGB")
        FourCC_ = FOURCC_GRGB;
    else if (FourCCName_ == "UYVY")
        FourCC_ = FOURCC_UYVY;
    else if (FourCCName_ == "YUY2")
        FourCC_ = FOURCC_YUY2;
    else if (FourCC == 36)
        FourCC_ = FOURCC_36;
    else if (FourCC == 110)
        FourCC_ = FOURCC_110;
    else if (FourCC == 111)
        FourCC_ = FOURCC_111;
    else if (FourCC == 112)
        FourCC_ = FOURCC_112;
    else if (FourCC == 113)
        FourCC_ = FOURCC_113;
    else if (FourCC == 114)
        FourCC_ = FOURCC_114;
    else if (FourCC == 115)
        FourCC_ = FOURCC_115;
    else if (FourCC == 116)
        FourCC_ = FOURCC_116;
    else if (FourCC == 117)
        FourCC_ = FOURCC_117;
    else if (FourCC != 0)
    {
        io::Log::error("Unknown FourCC type in DDS file (" + io::stringc(FourCC) + ")");
        return false;
    }
    else
        FourCC_ = FOURCC_NONE;
    
    return true;
}

bool ImageLoaderDDS::readBody()
{
    return
        ( ( FourCC_ >= FOURCC_DXT1 && FourCC_ <= FOURCC_DXT5 ) )
        ? readCompressed() : readUncompressed();
}

bool ImageLoaderDDS::readUncompressed()
{
    /* Read the bitmap image buffer */
    File_->readBuffer(TexData_->ImageBuffer, ImageBufferSize_);
    
    /* Flip the image colors (BGR to RGB) */
    ImageConverter::flipImageColors(
        TexData_->ImageBuffer, MainHeader_.Width, MainHeader_.Height, TexData_->FormatSize
    );
    
    return true;
}

bool ImageLoaderDDS::readCompressed()
{
    u32 CompressedBufferSize = MainHeader_.Width * MainHeader_.Height;
    
    if (FourCC_ == FOURCC_DXT1)
        CompressedBufferSize /= 2;
    
    if (isDepth_ && MainHeader_.Depth > 0)
        CompressedBufferSize *= MainHeader_.Depth;
    
    /* Allocate image buffer with the compressed data */
    u8* CompressedBuffer = new u8[CompressedBufferSize];
    u8* BufferAddr = CompressedBuffer;
    
    /* Read the compressed image buffer */
    File_->readBuffer(CompressedBuffer, CompressedBufferSize);
    
    /* Decompress image buffer */
    s32 x, y, sx, sy, i, j, k;
    
    u32 BitAlpha[2];
    u8 RealAlpha[16] = { 0 };
    
    u16 BitColor[2];
    video::color RealColor[4];
    
    for (y = 0; y < MainHeader_.Height / 4; ++y)
    {
        for (x = 0; x < MainHeader_.Width / 4; ++x)
        {
            if (FourCC_ == FOURCC_DXT2 || FourCC_ == FOURCC_DXT3)
            {
                /* Get the 16 4-bit alpha channels and store it in two UINTs */
                BitAlpha[0] = *((u32*)BufferAddr); BufferAddr += 4;
                BitAlpha[1] = *((u32*)BufferAddr); BufferAddr += 4;
                
                /* Convert the 4 bit alpha to real alpha */
                for (k = 0; k < 8; ++k)
                {
                    RealAlpha[    k] = get4BitAlpha(BitAlpha[0]);
                    RealAlpha[8 + k] = get4BitAlpha(BitAlpha[1]);
                }
            }
            
            /* Get the two 16 bit colors */
            BitColor[0] = *((u16*)BufferAddr); BufferAddr += 2;
            BitColor[1] = *((u16*)BufferAddr); BufferAddr += 2;
            
            /* Convert the 16 bit color to real colors */
            RealColor[0] = get16BitColor(BitColor[0]);
            RealColor[1] = get16BitColor(BitColor[1]);
            
            /* Get the next two colors by interpolating between the first two colors */
            RealColor[2] = getInterpolatedColor(RealColor[0], RealColor[1]);
            RealColor[3] = getInterpolatedColor(RealColor[1], RealColor[0]);
            
            /* Get the pixel data bit field */
            s32 PixelBitField = *((u32*)BufferAddr); BufferAddr += 4;
            
            /* Pass all pixels in the 4x4 block */
            for (sy = 0; sy < 4; ++sy)
            {
                for (sx = 0; sx < 4; ++sx)
                {
                    i = ( ((y << 2) + sy) * MainHeader_.Width + ((x << 2) + sx) )*TexData_->FormatSize;
                    
                    j = (PixelBitField & 0x00000003);
                    PixelBitField >>= 2;
                    
                    TexData_->ImageBuffer[i+0] = RealColor[j].Red;
                    TexData_->ImageBuffer[i+1] = RealColor[j].Green;
                    TexData_->ImageBuffer[i+2] = RealColor[j].Blue;
                    
                    if (isAlpha_ && TexData_->FormatSize == 4)
                    {
                        if (FourCC_ == FOURCC_DXT2 || FourCC_ == FOURCC_DXT3)
                            TexData_->ImageBuffer[i+3] = RealAlpha[(sy << 2) + sx];
                        else
                            TexData_->ImageBuffer[i+3] = RealColor[j].Alpha;
                    }
                }
            }
        }
    }
    
    /* Delete the compressed image buffer */
    delete [] CompressedBuffer;
    
    return true;
}

video::color ImageLoaderDDS::get16BitColor(u16 Color) const
{
    return video::color(
        ((Color >> 12) & 0x000F) * 256 / 16,
        ((Color >>  6) & 0x001F) * 256 / 32,
        ((Color >>  1) & 0x000F) * 256 / 16
    );
}

video::color ImageLoaderDDS::getInterpolatedColor(const video::color &ColorA, const video::color &ColorB) const
{
    return video::color(
        ColorA.Red      > ColorB.Red    ? calcColorProc1(ColorA.Red     , ColorB.Red    ) : calcColorProc2(ColorA.Red   , ColorB.Red    ),
        ColorA.Green    > ColorB.Green  ? calcColorProc1(ColorA.Green   , ColorB.Green  ) : calcColorProc2(ColorA.Green , ColorB.Green  ),
        ColorA.Blue     > ColorB.Blue   ? calcColorProc1(ColorA.Blue    , ColorB.Blue   ) : calcColorProc2(ColorA.Blue  , ColorB.Blue   )
    );
}

u8 ImageLoaderDDS::calcColorProc1(s32 c0, s32 c1) const
{
    return (u8)( ( 2 * c0 + c1 + 1 ) / 3 );
}
u8 ImageLoaderDDS::calcColorProc2(s32 c0, s32 c1) const
{
    return (u8)( ( c0 + c1 ) / 2 );
}

u8 ImageLoaderDDS::get4BitAlpha(u32 &BitAlpha) const
{
    u8 Alpha = (BitAlpha & 0x0000000F) * 256 / 16;
    BitAlpha >>= 4;
    return Alpha;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
