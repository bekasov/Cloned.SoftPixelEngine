/*
 * File object handler (HDD - HardDiskDrive) header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_FILE_HDD_H__
#define __SP_INPUTOUTPUT_FILE_HDD_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputFile.hpp"

#include <fstream>


namespace sp
{
namespace io
{


class SP_EXPORT FilePhysical : public File
{
    
    public:
        
        FilePhysical();
        ~FilePhysical();
        
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
        
        /* Static functions */
        
        static bool findFile(const io::stringc &Filename);
        
    private:
        
        /* Functions */
        
        static io::stringc getAppRootPath();
        
        /* Members */
        
        mutable std::fstream Stream_;
        
};


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
