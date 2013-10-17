/*
 * Format writer file
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spsFileWriter.h"


namespace sps
{


SpFileWriter::SpFileWriter()
{
}
SpFileWriter::~SpFileWriter()
{
}

bool SpFileWriter::Open(const std::string &Filename)
{   
    /* Open file stream for writing in binary mode */
    Stream_.open(Filename.c_str(), std::ios_base::out | std::ios_base::binary);
    return !Stream_.fail();
}

void SpFileWriter::Close()
{
    Stream_.close();
}

void SpFileWriter::Write(const void* Buffer, uint32 Size)
{
    Stream_.write(static_cast<const byte*>(Buffer), static_cast<std::streamsize>(Size));
}

void SpFileWriter::WriteString(const std::string &Str)
{
    const uint32 Len = static_cast<uint32>(Str.size());

    Write<uint32>(Len);

    if (Len)
        Write(Str.c_str(), Len);
}

void SpFileWriter::Seek(int32 Position)
{
    Stream_.seekp(static_cast<std::streampos>(Position));
}

void SpFileWriter::Seek(const EFileSeekModes Type, int32 Offset)
{
    switch (Type)
    {
        case SEEKMODE_BEGIN:
            Stream_.seekp(static_cast<std::streamoff>(Offset), std::ios_base::beg);
            break;
            
        case SEEKMODE_CURRENT:
            Stream_.seekp(static_cast<std::streamoff>(Offset), std::ios_base::cur);
            break;
            
        case SEEKMODE_END:
            Stream_.seekp(static_cast<std::streamoff>(Offset), std::ios_base::end);
            break;
    }
}

int32 SpFileWriter::GetPosition()
{
    return static_cast<int32>(Stream_.tellp());
}


} // /namespace sps



// ================================================================================
