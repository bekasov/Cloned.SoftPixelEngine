/*
 * File interface file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spInputOutputFile.hpp"


namespace sp
{
namespace io
{


File::File(const EFileTypes Type) :
    Type_       (Type           ),
    Permission_ (FILE_UNDEFINED )
{
}
File::~File()
{
}

s32 File::writeString(const stringc &Str, u32 Count)
{
    return writeBuffer(Str.c_str(), Str.size(), Count);
}

s32 File::writeStringN(const stringc &Str, u32 Count)
{
    s32 Result = 0;
    #ifdef SP_PLATFORM_WINDOWS
    const c8 nlChar1 = 13;
    #endif
    const c8 nlChar2 = 10;
    
    for (u32 i = 0; i < Count; ++i)
    {
        Result += writeBuffer(Str.c_str(), Str.size());
        
        #ifdef SP_PLATFORM_WINDOWS
        Result += writeBuffer(&nlChar1, 1);
        #endif
        
        Result += writeBuffer(&nlChar2, 1);
    }
    
    return Result;
}

s32 File::writeStringC(const stringc &Str, u32 Count)
{
    s32 Result = 0;
    const c8 CharZero = 0;
    
    for (u32 i = 0; i < Count; ++i)
    {
        Result += writeBuffer(Str.c_str(), Str.size());
        Result += writeBuffer(&CharZero, 1);
    }
    
    return Result;
}

s32 File::writeStringData(const stringc &Str, u32 Count)
{
    s32 Result = 0;
    const u32 Len = Str.size();
    
    for (u32 i = 0; i < Count; ++i)
    {
        Result += writeBuffer(&Len, sizeof(Len));
        Result += writeBuffer(Str.c_str(), Len);
    }
    
    return Result;
}

stringc File::readString(s32 Length) const
{
    c8* CharStr = new c8[Length + 1];
    
    readBuffer(CharStr, Length);
    CharStr[Length] = 0;
    
    io::stringc Str = CharStr;
    
    delete [] CharStr;
    
    return Str;
}

stringc File::readString(bool BreakPrompt) const
{
    io::stringc Str;
    c8 CurChar = 0;
    
    const c8 nlChar1 = 13, nlChar2 = 10;
    
    while (!isEOF())
    {
        CurChar = readValue<c8>();
        
        if (isEOF())//!!!
            break;
        
        if (CurChar == nlChar1)
        {
            if (BreakPrompt)
                break;
            else
                continue;
        }
        if (CurChar == nlChar2)
            break;
        
        Str += CurChar;
    }
    
    return Str;
}

stringc File::readStringC() const
{
    io::stringc Str;
    c8 CurChar;
    
    while (!isEOF())
    {
        readBuffer(&CurChar, sizeof(CurChar));
        
        if (CurChar == '\0')
            break;
        
        Str += CurChar;
    }
    
    return Str;
}

stringc File::readStringData() const
{
    u32 Len = 0;
    
    readBuffer(&Len, sizeof(Len));
    
    c8* CharStr = new c8[Len + 1];
    
    readBuffer(CharStr, Len);
    CharStr[Len] = 0;
    
    io::stringc Str = CharStr;
    
    delete [] CharStr;
    
    return Str;
}

void File::writeColor(const video::color &Color)
{
    writeBuffer(&Color.Red, sizeof(video::color));
}

video::color File::readColor() const
{
    video::color Color;
    readBuffer(&Color.Red, sizeof(video::color));
    return Color;
}


} // /namespace io

} // /namespace sp



// ================================================================================
