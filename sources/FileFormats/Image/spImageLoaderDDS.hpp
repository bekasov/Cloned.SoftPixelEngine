/*
 * Image loader DDS header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGELOADER_DDS_H__
#define __SP_IMAGELOADER_DDS_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_DDS


#include "FileFormats/Image/spImageFormatInterfaces.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT ImageLoaderDDS : public ImageLoader
{
    
    public:
        
        ImageLoaderDDS(io::File* File);
        ~ImageLoaderDDS();
        
        SImageDataRead* loadImageData();
        
    private:
        
        /* Enumerations */
        
        enum EFourCCTypesDDS
        {
            FOURCC_NONE,
            FOURCC_DXT1,
            FOURCC_DXT2,
            FOURCC_DXT3,
            FOURCC_DXT4,
            FOURCC_DXT5,
            FOURCC_DX10,
            FOURCC_BC4U,
            FOURCC_BC4S,
            FOURCC_BC5S,
            FOURCC_ATI2,
            FOURCC_RGBG,
            FOURCC_GRGB,
            FOURCC_UYVY,
            FOURCC_YUY2,
            FOURCC_36,
            FOURCC_110,
            FOURCC_111,
            FOURCC_112,
            FOURCC_113,
            FOURCC_114,
            FOURCC_115,
            FOURCC_116,
            FOURCC_117,
        };
        
        enum EImageFlagsDDS
        {
            // Main flags
            DDSFLAG_MIPMAPS     = 0x00020000,
            DDSFLAG_DEPTH       = 0x00800000,
            
            // Format flags
            DDSFLAG_ALPHA       = 0x00000001,
            DDSFLAG_COMPRESSED  = 0x00000004,
            
            // Cube map flags
            DDSFLAG_CUBEMAP     = 0x00000200,
        };
        
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
        
        struct SPixelFormatDDS
        {
            s32 StructSize;
            s32 Flags;
            s32 FourCC;
            s32 RGBBitCount;
            s32 RBitMask;
            s32 GBitMask;
            s32 BBitMask;
            s32 ABitMask;
        }
        SP_PACK_STRUCT;
        
        struct SHeaderDDS
        {
            s32 StructSize;
            s32 Flags;
            s32 Height;
            s32 Width;
            s32 Pitch;
            s32 Depth;
            s32 MipMapCount;
            s32 Reserved1[11];
            SPixelFormatDDS Format;
            s32 SurfaceFlags;
            s32 CubeMapFlags;
            s32 Reserved2[3];
        }
        SP_PACK_STRUCT;
        
        struct SHeaderDX10DDS
        {
            s32 Format;
            s32 Dimension;
            u32 MiscFlag;
            u32 ArraySize;
            u32 Reserved;
        }
        SP_PACK_STRUCT;
        
        #ifdef _MSC_VER
        #	pragma pack(pop, packing)
        #endif
        
        #undef SP_PACK_STRUCT
        
        /* Functions */
        
        bool readHeader();
        bool readBody();
        
        bool updateFourCCType();
        
        bool readUncompressed();
        bool readCompressed();
        
        video::color get16BitColor(u16 Color) const;
        video::color getInterpolatedColor(const video::color &ColorA, const video::color &ColorB) const;
        
        u8 calcColorProc1(s32 c0, s32 c1) const;
        u8 calcColorProc2(s32 c0, s32 c1) const;
        
        u8 get4BitAlpha(u32 &BitAlpha) const;
        
        /* Members */
        
        SHeaderDDS MainHeader_;
        SHeaderDX10DDS MainHeaderEx_;
        
        bool isHeaderDX10_;
        bool isMipMapped_;
        bool isDepth_;
        bool isAlpha_;
        bool isCompressed_;
        bool isCubeMap_;
        
        EFourCCTypesDDS FourCC_;
        io::stringc FourCCName_;
        
        SImageDataRead* TexData_;
        u32 ImageBufferSize_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
