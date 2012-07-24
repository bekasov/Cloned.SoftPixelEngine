/*
 * Format reader file
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spsFileReader.h"


namespace sps
{


SpFileReader::SpFileReader() :
    Size_(0)
{
}
SpFileReader::~SpFileReader()
{
}

bool SpFileReader::Open(const std::string &Filename)
{   
    /* Open file stream for reading in binary mode */
    Stream_.open(Filename.c_str(), std::ios_base::in | std::ios_base::binary);
    
    if (Stream_.fail())
        return false;
    
    /* Get file size */
    Stream_.seekg(0, std::ios::end);
    Size_ = GetPosition();
    Stream_.seekg(0);
    
    return true;
}

void SpFileReader::Close()
{
    Stream_.close();
}

void SpFileReader::Read(void* Buffer, uint32 Size)
{
    Stream_.read(static_cast<byte*>(Buffer), static_cast<std::streamsize>(Size));
}

bool SpFileReader::ReadString(std::string &Str)
{
    /* Read string length */
    uint32 Len = Read<uint32>();
    
    if (Len == 0)
    {
        Str = "";
        return true;
    }
    
    /* Check if string is too long */
    if (Len + GetPosition() >= Size_)
        return false;
    
    /* Read string characters */
    Str.resize(Len);
    Read(&Str[0], Len);
    
    return true;
}

void SpFileReader::Seek(int32 Position)
{
    Stream_.seekg(static_cast<std::streampos>(Position));
}

void SpFileReader::Seek(const EFileSeekModes Type, int32 Offset)
{
    switch (Type)
    {
        case SEEKMODE_BEGIN:
            Stream_.seekg(static_cast<std::streamoff>(Offset), std::ios_base::beg);
            break;
            
        case SEEKMODE_CURRENT:
            Stream_.seekg(static_cast<std::streamoff>(Offset), std::ios_base::cur);
            break;
            
        case SEEKMODE_END:
            Stream_.seekg(static_cast<std::streamoff>(Offset), std::ios_base::end);
            break;
    }
}

int32 SpFileReader::GetPosition()
{
    return static_cast<uint32>(Stream_.tellg());
}

bool SpFileReader::IsEOF() const
{
    return Stream_.eof();
}


} // /namespace sps



// ================================================================================
