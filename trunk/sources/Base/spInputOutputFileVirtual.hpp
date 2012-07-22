/*
 * File object handler header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_FILE_RAM_H__
#define __SP_INPUTOUTPUT_FILE_RAM_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputFile.hpp"
#include "Base/spDimensionUniversalBuffer.hpp"


namespace sp
{
namespace io
{


class SP_EXPORT FileVirtual : public File
{
    
    public:
        
        FileVirtual();
        FileVirtual(const io::stringc &Filename);
        ~FileVirtual();
        
        /* Functions */
        
        bool open(const io::stringc &Filename, const EFilePermission Permission = FILE_READWRITE);
        void close();
        
        s32 writeBuffer(const void* Buffer, u32 Size, u32 Count = 1);
        s32 readBuffer(void* Buffer, u32 Size, u32 Count = 1) const;
        
        void setSeek(s32 Pos, const EFileSeekTypes PosType = FILEPOS_BEGIN);
        s32 getSeek() const;
        
        bool isEOF() const;
        
        u32 getSize() const;
        void* getHandle();
        
        bool opened() const;
        
        /* Inline functions */
        
        inline const dim::UniversalBuffer& getBuffer() const
        {
            return Buffer_;
        }
        inline dim::UniversalBuffer& getBuffer()
        {
            return Buffer_;
        }
        
    private:
        
        /* Members */
        
        dim::UniversalBuffer Buffer_;
        mutable s32 Pos_;
        
        bool isOpened_;
        bool hasWriteAccess_, hasReadAccess_;
        
};


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
