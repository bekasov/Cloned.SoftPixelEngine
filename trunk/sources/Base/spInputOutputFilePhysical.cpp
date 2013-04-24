/*
 * File object handler (HDD - HardDiskDrive) file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spInputOutputFilePhysical.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "Base/spInputOutputLog.hpp"

#ifdef SP_PLATFORM_IOS
#   include <CoreFoundation/CoreFoundation.h>
#endif

#include <stdio.h>


namespace sp
{
namespace io
{


FilePhysical::FilePhysical() :
    File(FILE_PHYSICAL)
{
}
FilePhysical::~FilePhysical()
{
    close();
}

bool FilePhysical::open(const io::stringc &Filename, const EFilePermission Permission)
{
    /* Close file if still opened */
    close();
    
    /* Update filename and permission */
    if (Filename.size() && Filename[0] == '/')
        Filename_ = Filename;
    else
        Filename_ = FilePhysical::getAppRootPath() + Filename;
    
    Permission_ = Permission;
    
    /* Get file access mode */
    std::ios_base::openmode Mode = std::fstream::binary;
    
    switch (Permission)
    {
        case FILE_UNDEFINED:
            Log::error("Open file: \"" + Filename + "\" as undefined is not possible");
            return false;
            
        case FILE_READ:
            Mode |= std::fstream::in; break;
        case FILE_WRITE:
            Mode |= std::fstream::out; break;
        case FILE_WRITE_APPEND:
            Mode |= std::fstream::out | std::fstream::app; break;
        case FILE_READWRITE:
            Mode |= std::fstream::in | std::fstream::out; break;
        case FILE_READWRITE_APPEND:
            Mode |= std::fstream::in | std::fstream::out | std::fstream::app; break;
    }
    
    /* Open file stream and check for error */
    Stream_.open(Filename_.c_str(), Mode);
    
    if (!Stream_.is_open())
    {
        Log::error("Could not open file: \"" + Filename + "\"");
        return false;
    }
    
    return true;
}
void FilePhysical::close()
{
    if (Stream_.is_open())
        Stream_.close();
}

stringc FilePhysical::readString(bool BreakPrompt) const
{
    stringc Str;
    std::getline(Stream_, Str.str());
    return Str;
}

s32 FilePhysical::writeBuffer(const void* Buffer, u32 Size, u32 Count)
{
    /* Check for valid data */
    if (!Buffer || !Size || !Count || !opened())
        return 0;
    
    /* Write buffer into file */
    try
    {
        Stream_.write(static_cast<const c8*>(Buffer), Size * Count);
    }
    catch (std::ios_base::failure &e)
    {
        Log::error("< IO Base Failure > exception thrown: " + io::stringc(e.what()));
        return -1;
    }
    
    /* Return count of written bytes */
    return static_cast<s32>(Count * Size);
}

s32 FilePhysical::readBuffer(void* Buffer, u32 Size, u32 Count) const
{
    /* Check for valid data */
    if (!Buffer || !Size || !Count || !opened())
        return 0;
    
    /* Read buffer out of file */
    try
    {
        Stream_.read(static_cast<c8*>(Buffer), Size * Count);
    }
    catch (std::ios_base::failure &e)
    {
        Log::error("< IO Base Failure > exception thrown: " + io::stringc(e.what()));
        return -1;
    }
    
    /* Return count of read bytes */
    return static_cast<s32>(Count * Size);
}

void FilePhysical::setSeek(s32 Pos, const EFileSeekTypes PosType)
{
    std::ios_base::seekdir OffsetDir = std::ios_base::beg;
    
    switch (PosType)
    {
        case FILEPOS_BEGIN:
            OffsetDir = std::ios_base::beg; break;
        case FILEPOS_CURRENT:
            OffsetDir = std::ios_base::cur; break;
        case FILEPOS_END:
            OffsetDir = std::ios_base::end; break;
    }
    
    if (hasReadAccess())
        Stream_.seekg(Pos, OffsetDir);
    if (hasWriteAccess())
        Stream_.seekp(Pos, OffsetDir);
}
s32 FilePhysical::getSeek() const
{
    if (hasReadAccess())
        return static_cast<s32>(Stream_.tellg());
    else if (hasWriteAccess())
        return static_cast<s32>(Stream_.tellp());
    return 0;
}

bool FilePhysical::isEOF() const
{
    return Stream_.eof();
}

u32 FilePhysical::getSize() const
{
    u32 Size = 0;
    FILE* TempFile = fopen(Filename_.c_str(), "rb");
    
    if (TempFile)
    {
        fseek(TempFile, 0, SEEK_END);
        Size = ftell(TempFile);
        fclose(TempFile);
    }
    
    return Size;
}

void* FilePhysical::getHandle()
{
    return &Stream_;
}

bool FilePhysical::opened() const
{
    return Stream_.is_open();
}

bool FilePhysical::findFile(const io::stringc &Filename)
{
    FILE* TempFile = fopen((getAppRootPath() + Filename).c_str(), "rb");
    
    if (TempFile)
    {
        fclose(TempFile);
        return true;
    }
    
    return false;
}


/*
 * ======= Private: =======
 */

io::stringc FilePhysical::getAppRootPath()
{
    #ifdef SP_PLATFORM_IOS
    
    c8 BundlePath[1024];
    
    CFURLRef URLRef = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    
    if (!URLRef)
        return "";
    
    CFURLGetFileSystemRepresentation(URLRef, true, (UInt8*)BundlePath, sizeof(BundlePath));
    CFRelease(URLRef);
    
    return io::stringc(BundlePath) + "/";
    
    #else
    
    return "";
    
    #endif
}


} // /namespace io

} // /namespace sp



// ================================================================================
