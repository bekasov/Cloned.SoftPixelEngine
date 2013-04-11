/*
 * File system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spInputOutputFileSystem.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#endif


namespace sp
{
namespace io
{


#if defined(SP_PLATFORM_ANDROID)
EResourceConfigs FileSystem::ResourceConfig_ = RESOURCE_ASSET;
#else
EResourceConfigs FileSystem::ResourceConfig_ = RESOURCE_PHYSICAL;
#endif

FileSystem::FileSystem()
{
}
FileSystem::~FileSystem()
{
    MemoryManager::deleteList(FileList_);
}

File* FileSystem::openFile(const EFileTypes Type)
{
    File* NewFile = 0;
    
    switch (Type)
    {
        case FILE_PHYSICAL:
            NewFile = MemoryManager::createMemory<FilePhysical>("io::FilePhysical (io::FileSystem)"); break;
        case FILE_VIRTUAL:
            NewFile = MemoryManager::createMemory<FileVirtual>("io::FileVirtual (io::FileSystem)"); break;
        default:
            break;
    }
    
    if (NewFile)
        FileList_.push_back(NewFile);
    
    return NewFile;
}

FilePhysical* FileSystem::openFile(const stringc &Filename, const EFilePermission Permission)
{
    FilePhysical* NewFile = MemoryManager::createMemory<FilePhysical>("io::FilePhysical (io::FileSystem)");
    
    if (!NewFile->open(Filename, Permission))
    {
        MemoryManager::deleteMemory(NewFile);
        return 0;
    }
    
    FileList_.push_back(NewFile);
    
    return NewFile;
}

FileVirtual* FileSystem::openFile(const EFilePermission Permission)
{
    FileVirtual* NewFile = MemoryManager::createMemory<FileVirtual>("io::FileVirtual (io::FileSystem)");
    FileList_.push_back(NewFile);
    
    NewFile->open("", Permission);
    
    return NewFile;
}

FileVirtual* FileSystem::readFile(const io::stringc &Filename)
{
    FileVirtual* NewFile = new FileVirtual(Filename);
    FileList_.push_back(NewFile);
    return NewFile;
}

#if defined(SP_PLATFORM_ANDROID)

FileAsset* FileSystem::readAsset(const stringc &Filename)
{
    FileAsset* NewFile = MemoryManager::createMemory<FileAsset>("io::FileAsset (io::FileSystem)");
    
    if (!NewFile->open(Filename, FILE_READ))
    {
        MemoryManager::deleteMemory(NewFile);
        return 0;
    }
    
    FileList_.push_back(NewFile);
    
    return NewFile;
}

#endif

File* FileSystem::readResourceFile(const stringc &Filename)
{
    #if defined(SP_PLATFORM_ANDROID)
    if (ResourceConfig_ == RESOURCE_ASSET)
        return readAsset(Filename);
    #endif
    return openFile(Filename, FILE_READ);
    //return readFile(Filename); // it's slower, wtf?!
}

io::stringc FileSystem::readFileString(const io::stringc &Filename) const
{
    io::stringc StringBuffer;
    
    FILE* TempFile = fopen(Filename.c_str(), "rb");
    
    if (TempFile)
    {
        /* Get file size */
        fseek(TempFile, 0, SEEK_END);
        const u32 Size = ftell(TempFile);
        fseek(TempFile, 0, SEEK_SET);
        
        if (Size)
        {
            /* Read whole file buffer into string */
            StringBuffer = io::stringc::space(Size);
            fread(&StringBuffer[0], 1, Size, TempFile);
        }
        
        fclose(TempFile);
    }
    else
        Log::error("Could not open file: \"" + Filename + "\"");
    
    return StringBuffer;
}

void FileSystem::closeFile(File* &FileObject)
{
    MemoryManager::removeElement(FileList_, FileObject, true);
}

bool FileSystem::findFile(const stringc &Filename) const
{
    #if defined(SP_PLATFORM_ANDROID)
    if (ResourceConfig_ == RESOURCE_ASSET)
        return FileAsset::findFile(Filename);
    #endif
    return FilePhysical::findFile(Filename);
}

u32 FileSystem::getFileSize(const stringc &Filename) const
{
    u32 Size = 0;
    FILE* TempFile = fopen(Filename.c_str(), "rb");
    
    if (TempFile)
    {
        fseek(TempFile, 0, SEEK_END);
        Size = ftell(TempFile);
        fclose(TempFile);
    }
    
    return Size;
}

bool FileSystem::createFile(const stringc &Filename)
{
    FILE* TempFile = fopen(Filename.c_str(), "w");
    
    if (TempFile)
        fclose(TempFile);
    
    return TempFile != 0;
}

bool FileSystem::deleteFile(const stringc &Filename)
{
    return remove(Filename.c_str()) == 0;
}
bool FileSystem::moveFile(const stringc &Filename, const stringc &NewFilename)
{
    return rename(Filename.c_str(), NewFilename.c_str()) == 0;
}

bool FileSystem::copyFile(const stringc &Filename, const stringc &NewFilename)
{
    std::ifstream ifs(Filename.c_str(), std::ios::binary);
    std::ofstream ofs(NewFilename.c_str(), std::ios::binary);
    
    ofs << ifs.rdbuf();
    
    return true;
}

#if defined(SP_PLATFORM_WINDOWS)

stringc FileSystem::getCurrentDirectory() const
{
    static const s32 cmdStrLen = 512;
    
    c8 path[cmdStrLen];
    GetCurrentDirectory(cmdStrLen, path);
    
    return stringc(path);
}
void FileSystem::setCurrentDirectory(const stringc &Path)
{
    SetCurrentDirectory(Path.c_str());
}

bool FileSystem::createDirectory(const stringc &Path)
{
    return CreateDirectory(Path.c_str(), 0) == TRUE;
}

bool FileSystem::deleteDirectory(const stringc &Path)
{
    return RemoveDirectory(Path.c_str()) == TRUE;
}

#elif defined(SP_PLATFORM_LINUX)

bool FileSystem::createDirectory(const stringc &Path)
{
    return false;
}

bool FileSystem::deleteDirectory(const stringc &Path)
{
    return !system(("rmdir " + Path).c_str());
}

#endif

void FileSystem::setResourceConfig(const EResourceConfigs Config)
{
    ResourceConfig_ = Config;
}
EResourceConfigs FileSystem::getResourceConfig()
{
    return ResourceConfig_;
}


} // /namespace io

} // /namespace sp



// ================================================================================
