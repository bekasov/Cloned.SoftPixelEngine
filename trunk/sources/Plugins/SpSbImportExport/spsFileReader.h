/*
 * File reader header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_FILE_READER_H__
#define __SPS_FILE_READER_H__


#include "spsCore.h"

#include <fstream>


namespace sps
{


class SpFileReader
{
    
    public:
        
        SpFileReader();
        ~SpFileReader();
        
        /* === Functions === */
        
        bool Open(const std::string &Filename);
        void Close();
        
        void Read(void* Buffer, uint32 Size);
        bool ReadString(std::string &Str);
        
        void Seek(int32 Position);
        void Seek(const EFileSeekModes Type, int32 Offset);
        
        int32 GetPosition();
        
        bool IsEOF() const;
        
        /* === Templates === */
        
        template <typename T> T Read()
        {
            T Buffer;
            Read(&Buffer, sizeof(T));
            return Buffer;
        }
        
    private:
        
        /* === Members === */
        
        std::ifstream Stream_;
        
        uint32 Size_;
        
};


} // /namespace sps


#endif



// ================================================================================
