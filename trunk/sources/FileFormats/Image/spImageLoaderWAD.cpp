/*
 * Image loader WAD file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Image/spImageLoaderWAD.hpp"

#ifdef SP_COMPILE_WITH_TEXLOADER_WAD


#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


ImageLoaderWAD::ImageLoaderWAD()
    : FileLoader_(0), File_(0)
{
    init();
}
ImageLoaderWAD::~ImageLoaderWAD()
{
    clear();
}

std::list<video::Texture*> ImageLoaderWAD::loadTextureList(const io::stringc &Filename)
{
    /* Reset the loader */
    reset();
    
    /* Information message */
    io::Log::message("Load WAD texture list: \"" + Filename + "\"");
    
    /* Open a new file */
    File_ = FileLoader_->openFile(Filename, io::FILE_READ);
    
    /* Check if the file has been opened successful */
    if (!File_)
        return TextureList_;
    
    /* Read the header */
    readHeader();
    
    /* Read the texture information */
    readTextureInfo();
    
    /* Read the textures */
    readTextures();
    
    /* Close the file */
    FileLoader_->closeFile(File_);
    
    /* Return the entity */
    return TextureList_;
}

void ImageLoaderWAD::init()
{
    /* Allocate a new file loader */
    FileLoader_ = new io::FileSystem();
}

void ImageLoaderWAD::clear()
{
    /* Delete the file loader */
    MemoryManager::deleteMemory(FileLoader_);
}

void ImageLoaderWAD::reset()
{
    /* Clear the lists */
    TextureInfoList_.clear();
    TextureList_.clear();
}

bool ImageLoaderWAD::readHeader()
{
    /* Read the header */
    File_->readBuffer(&Header_, sizeof(Header_));
    
    /* Check the magic identity */
    c8 Magic[5] = { 0 };
    memcpy(Magic, Header_.Magic, 4);
    
    if (io::stringc(Magic) != "WAD2" && io::stringc(Magic) != "WAD3")
    {
        /* Print an error message and return with a failure */
        io::Log::error("WAD file has incorrect identity");
        return false;
    }
    
    /* Exit the function and return true for success */
    return true;
}

void ImageLoaderWAD::readTextureInfo()
{
    /* Temporary variables */
    STextureWAD Texture;
    
    /* Set the offset */
    File_->setSeek(Header_.DirOffset);
    
    /* Read all texture information */
    for (u32 i = 0; i < Header_.CountTextures; ++i)
    {
        /* Read the vertex data and add texture information to the list */
        File_->readBuffer(&Texture, sizeof(Texture));
        TextureInfoList_.push_back(Texture);
    }
}

void ImageLoaderWAD::readTextures()
{
    /* Temporary variables */
    u8* ImageBuffer         = 0;
    u8* Palette             = 0;
    u8* FinalImageBuffer    = 0;
    
    c8 Name[17] = { 0 };
    
    SWallTextureBSP Texture;
    
    video::Texture* FinalTexture = 0;
    
    const s32 PaletteSize = 256 * 3;
    
    u32 ImageBufferSize, x, y, j;
    
    /* Loop for all textures */
    for (u32 i = 0; i < Header_.CountTextures; ++i)
    {
        /* Set the offset */
        File_->setSeek(TextureInfoList_[i].Offset);
        
        /* Read the BSP information */
        File_->readBuffer(&Texture, sizeof(SWallTextureBSP));
        
        /* Get the ANSI-C string name */
        memset(Name, 0, 17);
        memcpy(Name, Texture.Name, 16);
        
        /* Information message */
        io::Log::message("Load WAD texture: \"" + io::stringc(Name) + "\"");
        
        /* Set the offset */
        File_->setSeek(TextureInfoList_[i].Offset + Texture.Offset1);
        
        /* Compute the image data size */
        ImageBufferSize     = Texture.Width * Texture.Height;
        
        /* Allocate new image data for the palette */
        Palette             = new u8[PaletteSize];
        
        /* Allocate new image data */
        ImageBuffer         = new u8[ImageBufferSize];
        
        /* Allocate new final image data */
        FinalImageBuffer    = new u8[ImageBufferSize*3];
        
        /* Read the image data */
        File_->readBuffer(ImageBuffer, ImageBufferSize);
        
        /* Set the offset */
        File_->setSeek(
            TextureInfoList_[i].Offset + TextureInfoList_[i].UncompressedSize - PaletteSize - 2//4
        );
        
        //io::printMessage(FileLoader_->readValue<s16>(File_));
        
        /* Read the image data for the palette */
        File_->readBuffer(Palette, PaletteSize);
        
        /* Loop for image data */
        for (y = 0, j = 0; y < Texture.Height; ++y)
        {
            for (x = 0; x < Texture.Width; ++x, ++j)
            {
                FinalImageBuffer[j*3+0] = Palette[ ImageBuffer[j]*3 + 0 ];
                FinalImageBuffer[j*3+1] = Palette[ ImageBuffer[j]*3 + 1 ];
                FinalImageBuffer[j*3+2] = Palette[ ImageBuffer[j]*3 + 2 ];
            } // next pixel
        } // next scan line
        
        /* Create a new SoftPixel texture */
        FinalTexture = GlbRenderSys->createTexture(
            dim::size2di(Texture.Width, Texture.Height), video::PIXELFORMAT_RGB, ImageBuffer
        );
        FinalTexture->setName(Name);
        
        video::ImageBuffer* ImgBuffer = FinalTexture->getImageBuffer();
        
        /* Check for special attributes */
        switch (Name[0])
        {
            case '{':
            {
                video::color cr(Palette[255*3+0], Palette[255*3+1], Palette[255*3+2]);
                
                /* Determine which color-key must be used */
                if (cr != video::color(0, 0, 255))
                {
                    video::color cl(Palette[0], Palette[1], Palette[2]);
                    
                    /* Make color-key */
                    if (cl == video::color(255))
                        FinalTexture->setColorKeyAlpha(video::BLENDING_DARK);
                    else
                        FinalTexture->setColorKeyAlpha(video::BLENDING_BRIGHT);
                }
                else
                {
                    /* Make color-key */
                    FinalTexture->setColorKey(video::color(0, 0, 255, 0));
                    
                    /* Replace the blue color with black */
                    for (y = 0; y < Texture.Height; ++y)
                    {
                        for (x = 0; x < Texture.Width; ++x)
                        {
                            if (ImgBuffer->getPixelColor(dim::point2di(x, y)) == video::color(0, 0, 255, 0))
                                ImgBuffer->setPixelColor(dim::point2di(x, y), video::color(0, 0, 0, 0));
                        }
                    }
                    
                    /* Update the image data */
                    FinalTexture->updateImageBuffer();
                }
            }
            break;
        }
        
        //if (io::stringc(Name).left(5) == "glass")
        //    FinalTexture->setAlphaIntensity(128);
        
        /* Add the final texture to the texture list */
        TextureList_.push_back(FinalTexture);
        
        /* Delete the image buffer and palette */
        MemoryManager::deleteBuffer(ImageBuffer);
        MemoryManager::deleteBuffer(Palette);
    }
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
