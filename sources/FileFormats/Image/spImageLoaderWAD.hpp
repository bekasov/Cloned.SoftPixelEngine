/*
 * Image loader WAD header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IMAGELOADER_WAD_H__
#define __SP_IMAGELOADER_WAD_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_WAD


#include "RenderSystem/spTextureBase.hpp"

#include <vector>
#include <list>


namespace sp
{
namespace video
{


class SP_EXPORT ImageLoaderWAD
{
    
    public:
        
        ImageLoaderWAD();
        ~ImageLoaderWAD();
        
        std::list<Texture*> loadTextureList(const io::stringc &Filename);
        
    private:
        
        /* === Structures === */
        
        struct SHeaderWAD
        {
            c8 Magic[4];
            u32 CountTextures;
            u32 DirOffset;
        };
        
        struct STextureWAD
        {
            u32 Offset;
            u32 CompressedSize;
            u32 UncompressedSize;
            s8 Type;
            s8 CompressionType;
            s16 Padding;
            c8 Name[16];
        };
        
        struct SWallTextureBSP
        {
            c8 Name[16];
            u32 Width, Height;
            u32 Offset1;
            u32 Offset2;
            u32 Offset4;
            u32 Offset8;
        };
        
        /* === Functions === */
        
        void reset();
        
        bool readHeader();
        void readTextureInfo();
        void readTextures();
        
        /* === Members === */
        
        io::FileSystem FileSys_;
        io::File* File_;
        
        SHeaderWAD Header_;
        
        std::vector<STextureWAD> TextureInfoList_;
        std::list<Texture*> TextureList_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
