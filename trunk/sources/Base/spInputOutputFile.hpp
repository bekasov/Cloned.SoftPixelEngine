/*
 * File interface header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_FILE_H__
#define __SP_INPUTOUTPUT_FILE_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionQuaternion.hpp"
#include "Base/spDimensionMatrix4.hpp"
#include "Base/spMaterialColor.hpp"

#include <cstdlib>
#include <sstream>
#include <string>
#include <cstdio>


namespace sp
{
namespace io
{


//! File seek positions.
enum EFileSeekTypes
{
    FILEPOS_BEGIN   = SEEK_SET,
    FILEPOS_CURRENT = SEEK_CUR,
    FILEPOS_END     = SEEK_END
};

//! File access permissions.
enum EFilePermission
{
    FILE_UNDEFINED,         //!< Undefined permission when the file is not opened yet.
    FILE_READ,              //!< Only read access.
    FILE_WRITE,             //!< Only write access.
    FILE_WRITE_APPEND,      //!< Only write access. Previous content will be retained.
    FILE_READWRITE,         //!< Read- and write access.
    FILE_READWRITE_APPEND,  //!< Read- and write access. Previous content will be retained.
};

//! File object types.
enum EFileTypes
{
    FILE_PHYSICAL,  //!< Physical file stored on a HDD, SSD or Flash-Drive.
    FILE_VIRTUAL,   //!< Virtual file stored in RAM only.
    FILE_ASSET,     //!< Resource file with read access only for Android.
};


//! File base class.
class SP_EXPORT File
{
    
    public:
        
        virtual ~File();
        
        /* === File access functions === */
        
        /**
        Opens the specified file. This function guarantees that the previous opened file will always closed before open the new one.
        \param Filename: Specifies the file's name. This can be a relativ or absulte filename.
        \param Permission: Specifies the file access permission.
        \return True if the file could be opened successful.
        */
        virtual bool open(const io::stringc &Filename, const EFilePermission Permission = FILE_READWRITE) = 0;
        
        //! Closes the file.
        virtual void close() = 0;
        
        /* === Write functions === */
        
        //! Writes the given characters into the file.
        virtual s32 writeString(const stringc &Str, u32 Count = 1);
        
        //! Writes the given characters into the file with a new-line character at the end (on Windows '13' and '10'; on Linux '10').
        virtual s32 writeStringN(const stringc &Str, u32 Count = 1);
        
        //! Writes the given characters into the file in C-style (string terminated by zero).
        virtual s32 writeStringC(const stringc &Str, u32 Count = 1);
        
        //! Writes the given characters into the file where an integer (4 bytes) at the beginning specifies the string's length.
        virtual s32 writeStringData(const stringc &Str, u32 Count = 1);
        
        /* === Read functions === */
        
        //! Reads characters from the file where "Length" specifies the count of characters which are to be read.
        virtual stringc readString(s32 Length) const;
        
        //! Reads characters from the file where "BreakPrompt" specifies if reading breaks after a '13' or '10' has been read.
        virtual stringc readString(bool BreakPrompt = false) const;
        
        //! Reads characters from the file in C-style (until a zero has been read).
        virtual stringc readStringC() const;
        
        //! Reads characters from the file where the first four bytes (integer) specifies the count.
        virtual stringc readStringData() const;
        
        /* === Other reading/writing functions === */
        
        virtual s32 writeBuffer(const void* Buffer, u32 Size, u32 Count = 1) = 0;
        virtual s32 readBuffer(void* Buffer, u32 Size, u32 Count = 1) const = 0;
        
        virtual void writeColor(const video::color &Color);
        virtual video::color readColor() const;
        
        virtual void setSeek(s32 Pos, const EFileSeekTypes PosType = FILEPOS_BEGIN) = 0;
        virtual s32 getSeek() const = 0;
        
        //! Returns true if the file read position is at the End-Of-File.
        virtual bool isEOF() const = 0;
        
        //! Returns the file's size (in bytes).
        virtual u32 getSize() const = 0;
        
        /**
        Returns the file's handle.
        For FilePhysical objects this is a std::fstream* (C++ file stream),
        for FileVirtual objects it is the char* to the buffer (or rather array),
        for FileAsset objects it is an AAsset* (only for Android).
        */
        virtual void* getHandle() = 0;
        
        //! Returns true if the file is currently opened.
        virtual bool opened() const = 0;
        
        /* === Inline functions === */
        
        inline void setFilename(const io::stringc &Filename)
        {
            Filename_ = Filename;
        }
        inline stringc getFilename() const
        {
            return Filename_;
        }
        
        //! Returns the file type.
        inline EFileTypes getType() const
        {
            return Type_;
        }
        //! Returns the file permission (read/write access etc.).
        inline EFilePermission getPermission() const
        {
            return Permission_;
        }
        
        inline bool hasReadAccess() const
        {
            return Permission_ == FILE_READ || Permission_ == FILE_READWRITE || Permission_ == FILE_READWRITE_APPEND;
        }
        inline bool hasWriteAccess() const
        {
            return Permission_ == FILE_WRITE || Permission_ == FILE_WRITE_APPEND || Permission_ == FILE_READWRITE || Permission_ == FILE_READWRITE_APPEND;
        }
        
        //! Ignores the following 'Count' bytes. This is aquivalent to "setSeek(Count, FILEPOS_CURRENT)".
        inline void ignore(s32 Count)
        {
            setSeek(Count, FILEPOS_CURRENT);
        }
        
        /* === Templates === */
        
        template <typename T> s32 writeValue(const T &Value, u32 Count = 1)
        {
            return writeBuffer(&Value, sizeof(T), Count);
        }
        template <typename T> T readValue() const
        {
            T Value;
            readBuffer(&Value, sizeof(Value));
            return Value;
        }
        
        template <typename T> void writeVector(const dim::vector3d<T> &Vec)
        {
            writeBuffer(&Vec.X, sizeof(dim::vector3d<T>));
        }
        template <typename T> dim::vector3d<T> readVector() const
        {
            dim::vector3d<T> Vec;
            readBuffer(&Vec.X, sizeof(dim::vector3d<T>));
            return Vec;
        }
        
        template <typename T> void writeMatrix(const dim::matrix4<T> &Mat)
        {
            writeBuffer(Mat.getArray(), sizeof(dim::matrix4<T>));
        }
        template <typename T> dim::matrix4<T> readMatrix() const
        {
            dim::matrix4<T> Mat;
            readBuffer(&Mat[0], sizeof(dim::matrix4<T>));
            return Mat;
        }
        
        inline void writeQuaternion(const dim::quaternion &Quat)
        {
            writeBuffer(&Quat.X, sizeof(dim::quaternion));
        }
        inline dim::quaternion readQuaternion() const
        {
            dim::quaternion Quat;
            readBuffer(&Quat.X, sizeof(dim::quaternion));
            return Quat;
        }
        
    protected:
        
        File(const EFileTypes Type);
        
        /* Members */
        
        stringc Filename_;
        EFileTypes Type_;
        EFilePermission Permission_;
        
};


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
