/*
 * File object handler file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spInputOutputFileVirtual.hpp"
#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace io
{


FileVirtual::FileVirtual() :
    File            (FILE_VIRTUAL   ),
    Pos_            (0              ),
    isOpened_       (false          ),
    hasWriteAccess_ (false          ),
    hasReadAccess_  (false          )
{
}
FileVirtual::FileVirtual(const io::stringc &Filename) :
    File            (FILE_VIRTUAL   ),
    Pos_            (0              ),
    isOpened_       (false          ),
    hasWriteAccess_ (false          ),
    hasReadAccess_  (false          )
{
    Filename_ = Filename;
    
    /* Open file for reading the whole buffer */
    FILE* FileHandle = fopen(Filename.c_str(), "rb");
    
    if (!FileHandle)
    {
        Log::error("Could not open file: \"" + Filename + "\"");
        return;
    }
    
    /* Get file size */
    fseek(FileHandle, 0, SEEK_END);
    const u32 Size = static_cast<u32>(ftell(FileHandle));
    fseek(FileHandle, 0, SEEK_SET);
    
    /* Read whole file into buffer */
    Buffer_.setSize(Size);
    
    fread(Buffer_.getArray(), Size, 1, FileHandle);
    fclose(FileHandle);
    
    isOpened_       = true;
    hasReadAccess_  = true;
    Permission_     = FILE_READ;
}
FileVirtual::~FileVirtual()
{
}

bool FileVirtual::open(const io::stringc &Filename, const EFilePermission Permission)
{
    /* Close file if still opened */
    close();
    
    /* Update filename and permission */
    Filename_   = Filename;
    Permission_ = Permission;
    
    /* Get file access mode */
    switch (Permission_)
    {
        case FILE_UNDEFINED:
            Log::error("Open file: \"" + Filename + "\" as undefined is not possible");
            return false;
            
        case FILE_READ:
            hasReadAccess_  = true;
            hasWriteAccess_ = false;
            Pos_            = 0;
            break;
            
        case FILE_WRITE:
            hasReadAccess_  = false;
            hasWriteAccess_ = true;
            Pos_            = 0;
            Buffer_.clear();
            break;
            
        case FILE_WRITE_APPEND:
            hasReadAccess_  = false;
            hasWriteAccess_ = true;
            break;
            
        case FILE_READWRITE:
            hasReadAccess_  = true;
            hasWriteAccess_ = true;
            Pos_            = 0;
            Buffer_.clear();
            break;
            
        case FILE_READWRITE_APPEND:
            hasReadAccess_  = true;
            hasWriteAccess_ = true;
            break;
    }
    
    isOpened_ = true;
    
    return true;
}
void FileVirtual::close()
{
    isOpened_       = false;
    hasReadAccess_  = false;
    hasWriteAccess_ = false;
    Pos_            = 0;
}

s32 FileVirtual::writeBuffer(const void* Buffer, u32 Size, u32 Count)
{
    /* Check for valid data */
    if (!Buffer || !Size || !Count || !isOpened_ || !hasWriteAccess_)
        return 0;
    
    /* Write buffer into file */
    for (u32 i = 0; i < Count; ++i)
    {
        const u32 PrevSize = Buffer_.getSize();
        Buffer_.setSize(PrevSize + Size);
        Buffer_.setBuffer(PrevSize, Buffer, Size);
    }
    
    /* Boost file position */
    Pos_ += Size * Count;
    
    /* Return count of written bytes */
    return static_cast<s32>(Size * Count);
}

s32 FileVirtual::readBuffer(void* Buffer, u32 Size, u32 Count) const
{
    /* Check for valid data */
    if (!Buffer || !Size || !Count || !isOpened_ || !hasReadAccess_)
        return 0;
    
    /* Read buffer out of file */
    Buffer_.getBuffer(Pos_, Buffer, Size * Count);
    
    /* Boost file position */
    Pos_ += Size * Count;
    
    /* Return count of read bytes */
    return static_cast<s32>(Size * Count);
}

void FileVirtual::setSeek(s32 Pos, const EFileSeekTypes PosType)
{
    if (!Buffer_.getSize())
        return;
    
    switch (PosType)
    {
        case FILEPOS_BEGIN:
            Pos_ = Pos; break;
        case FILEPOS_CURRENT:
            Pos_ += Pos; break;
        case FILEPOS_END:
            Pos_ = Buffer_.getSize() - 1 - Pos; break;
    }
    
    math::Clamp(Pos_, 0, static_cast<s32>(Buffer_.getSize() - 1));
}
s32 FileVirtual::getSeek() const
{
    return Pos_;
}

bool FileVirtual::isEOF() const
{
    return isOpened_ && static_cast<u32>(Pos_) >= Buffer_.getSize();
}

u32 FileVirtual::getSize() const
{
    return Buffer_.getSize();
}
void* FileVirtual::getHandle()
{
    return Buffer_.getArray();
}

bool FileVirtual::opened() const
{
    return isOpened_;
}


} // /namespace io

} // /namespace sp



// ================================================================================
