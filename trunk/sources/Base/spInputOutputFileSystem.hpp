/*
 * File system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_FILESYSTEM_H__
#define __SP_INPUTOUTPUT_FILESYSTEM_H__


#include "Base/spStandard.hpp"
#include "Base/spMemoryManagement.hpp"
#include "Base/spInputOutputFilePhysical.hpp"
#include "Base/spInputOutputFileVirtual.hpp"
#include "Base/spInputOutputFileAsset.hpp"
#include "Base/spInputOutputLog.hpp"

#include <list>


namespace sp
{
namespace io
{


enum EResourceConfigs
{
    RESOURCE_PHYSICAL,  //!< Resource files will be read from HDD, SSD or Flash-Drive.
    RESOURCE_ASSET,     //!< Resource files will be read from assets (only for Android).
};


/**
FileSystem can be used for easy file operations such as reading and writing from and to the disk.
Internally the ANSI C "FILE" pointer with its standard C file functions are used.
*/
class SP_EXPORT FileSystem
{
    
    public:
        
        FileSystem();
        ~FileSystem();
        
        //! Opens a new File object. To start reading or writing call "File::open".
        File* openFile(const EFileTypes Type);
        
        //! Opens a new FilePhysical object with the specified filename and the specified permission.
        FilePhysical* openFile(const stringc &Filename, const EFilePermission Permission = FILE_READWRITE);
        
        //! Opens a new FileVirtual object with the specified permission.
        FileVirtual* openFile(const EFilePermission Permission = FILE_READWRITE);
        
        //! Reads the whole specified file from HDD into the new FileVirtual's buffer.
        FileVirtual* readFile(const io::stringc &Filename);
        
        #if defined(SP_PLATFORM_ANDROID)
        //! Reads an asset (or rather resource file) for the Android platform.
        FileAsset* readAsset(const stringc &Filename);
        #endif
        
        /**
        Reads a resource file. This can be an asset or an HDD file which depends on the current configuration.
        \param Filename: Specifies the resource filename.
        \return Pointer to the new File object.
        \see {setResourceConfig}
        */
        File* readResourceFile(const stringc &Filename);
        
        //! Reads the whole file into the return string.
        io::stringc readFileString(const io::stringc &Filename) const;
        
        //! Closes and deletes the specified File object.
        void closeFile(File* &FileObject);
        
        //! Returns true if the specified file does exist on HDD (hard-disk drive).
        bool findFile(const stringc &Filename) const;
        
        //! Returns size of the specified file on hard-disk drive (in bytes).
        u32 getFileSize(const io::stringc &Filename) const;
        
        /**
        Creates a new file on HDD.
        \return True if succeed. Otherwise false.
        \note If this file already exist it will be cleared. Be careful!
        */
        bool createFile(const stringc &Filename);
        bool deleteFile(const stringc &Filename);
        bool copyFile(const stringc &Filename, const stringc &NewFilename);
        bool moveFile(const stringc &Filename, const stringc &NewFilename);
        
        /* Directories */
        
        //! Sets the new working directory. If you are handling files with relative paths be aware about the current working directory!
        void setCurrentDirectory(const stringc &Path);
        stringc getCurrentDirectory() const;
        
        bool createDirectory(const stringc &Path);
        bool deleteDirectory(const stringc &Path);
        
        /* Static functions */
        
        //! Sets the resource cofiguration. By default RESOURCE_ASSET for Android and RESOURCE_PHYSICAL for other OS.
        static void setResourceConfig(const EResourceConfigs Config);
        static EResourceConfigs getResourceConfig();
        
    private:
        
        /* Members */
        
        std::list<File*> FileList_;
        
        static EResourceConfigs ResourceConfig_;
        
};


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
