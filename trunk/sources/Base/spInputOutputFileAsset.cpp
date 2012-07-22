/*
 * File asset file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spInputOutputFileAsset.hpp"

#if defined(SP_PLATFORM_ANDROID)


#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace io
{


AAssetManager* FileAsset::AssetManager_ = 0;

FileAsset::FileAsset()
    : File(FILE_ASSET), Asset_(0)
{
    Permission_ = FILE_READ;
}
FileAsset::~FileAsset()
{
}

bool FileAsset::open(const io::stringc &Filename, const EFilePermission Permission)
{
    /* Check for correct permission */
    if (Permission != Permission_)
    {
        io::Log::error("No write access for resource files");
        return false;
    }
    
    /* Close file if still opened */
    close();
    
    /* Update filename and permission */
    Filename_   = Filename;
    Pos_        = 0;
    
    /* Open the asset resource file */
    Asset_ = AAssetManager_open(FileAsset::AssetManager_, Filename_.c_str(), AASSET_MODE_BUFFER);
    
    return opened();
}
void FileAsset::close()
{
    if (Asset_)
        AAsset_close(Asset_);
}

s32 FileAsset::writeBuffer(const void* Buffer, u32 Size, u32 Count)
{
    io::Log::error("No write access for assets");
    return 0;
}

s32 FileAsset::readBuffer(void* Buffer, u32 Size, u32 Count) const
{
    /* Check for valid data */
    if (!Buffer || !Size || !Count || !opened())
        return 0;
    
    /* Read buffer out of file */
    s32 Result = AAsset_read(Asset_, Buffer, Size * Count);
    
    /* Check for errors */
    if (Result < 0)
    {
        Log::error("Could not read buffer out of asset");
        return -1;
    }
    
    /* Boost seek position */
    Pos_ += Result;
    
    /* Return count of read bytes */
    return Result;
}

void FileAsset::setSeek(s32 Pos, const EFileSeekTypes PosType)
{
    /* Seek asset to the given position */
    s32 Result = AAsset_seek(
        Asset_, static_cast<off_t>(Pos), static_cast<s32>(PosType)
    );
    
    if (Result != -1)
        Pos_ = Result;
}
s32 FileAsset::getSeek() const
{
    return Pos_;
}

bool FileAsset::isEOF() const
{
    return getSeek() == getSize();
}

u32 FileAsset::getSize() const
{
    return static_cast<u32>(AAsset_getLength(Asset_));
}
void* FileAsset::getHandle()
{
    return Asset_;
}

bool FileAsset::opened() const
{
    return Asset_ != 0;
}

bool FileAsset::findFile(const io::stringc &Filename)
{
    if (Filename.size())
    {
        AAsset* Asset = AAssetManager_open(FileAsset::AssetManager_, Filename.c_str(), AASSET_MODE_BUFFER);
        
        if (Asset)
        {
            AAsset_close(Asset);
            return true;
        }
    }
    
    return false;
}


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
