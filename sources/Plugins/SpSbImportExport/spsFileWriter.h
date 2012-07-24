/*
 * File writer header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_FILE_WRITER_H__
#define __SPS_FILE_WRITER_H__


#include "spsCore.h"

#include <fstream>


namespace sps
{


class SpFileWriter
{
    
    public:
        
        SpFileWriter();
        ~SpFileWriter();
        
        /* === Functions === */
        
        bool Open(const std::string &Filename);
        void Close();
        
        void Write(const void* Buffer, uint32 Size);
        void WriteString(const std::string &Str);
        
        void Seek(int32 Position);
        void Seek(const EFileSeekModes Type, int32 Offset);
        
        int32 GetPosition();
        
        /* === Templates === */
        
        template <typename T> void Write(const T &Buffer)
        {
            Write(&Buffer, sizeof(T));
        }
        
    private:
        
        /* === Members === */
        
        std::ofstream Stream_;
        
};


} // /namespace sps


#endif



// ================================================================================
