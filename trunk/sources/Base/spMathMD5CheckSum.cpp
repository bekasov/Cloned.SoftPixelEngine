/*
 * MD5 check sum file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMathMD5CheckSum.hpp"


namespace sp
{
namespace math
{


MD5CheckSum::MD5CheckSum()
{
}
MD5CheckSum::MD5CheckSum(const void* Buffer, u32 Size)
{
    computeCheckSum(Buffer, Size);
}
MD5CheckSum::MD5CheckSum(const MD5CheckSum &Other) :
    CheckSum_(Other.CheckSum_)
{
}
MD5CheckSum::~MD5CheckSum()
{
}

bool MD5CheckSum::valid() const
{
    return CheckSum_.getHighWord() != 0 && CheckSum_.getLowWord() != 0;
}

io::stringc MD5CheckSum::getHexString() const
{
    io::stringc HexStr;
    
    HexStr += io::getHexString(CheckSum_.getHighWord());
    HexStr += io::getHexString(CheckSum_.getLowWord());
    
    return HexStr;
}


/*
 * ======= Private: =======
 */

void MD5CheckSum::computeCheckSum(const void* Buffer, u32 Size)
{
    
    //todo ...
    
}

UInt128 MD5CheckSum::getF(const UInt128 &B, const UInt128 &C, const UInt128 &D) const
{
    /* F(B, C, D) := (B & C) | (~B & D) */
    UInt128 Temp0(B);
    Temp0 &= C;
    
    UInt128 Temp1(B);
    Temp1.invert();
    Temp1 &= D;
    
    Temp0 |= Temp1;
    
    return Temp0;
}

UInt128 MD5CheckSum::getG(const UInt128 &B, const UInt128 &C, const UInt128 &D) const
{
    /* G(B, C, D) := (B & D) | (C & ~D) */
    UInt128 Temp0(B);
    Temp0 &= D;
    
    UInt128 Temp1(D);
    Temp1.invert();
    Temp1 &= C;
    
    Temp0 |= Temp1;
    
    return Temp0;
}

UInt128 MD5CheckSum::getH(const UInt128 &B, const UInt128 &C, const UInt128 &D) const
{
    /* H(B, C, D) := B ^ C ^ D */
    UInt128 Temp0(B);
    
    Temp0 ^= C;
    Temp0 ^= D;
    
    return Temp0;
}

UInt128 MD5CheckSum::getI(const UInt128 &B, const UInt128 &C, const UInt128 &D) const
{
    /* I(B, C, D) := C ^ (B | ~D) */
    UInt128 Temp0(D);
    
    Temp0.invert();
    Temp0 &= B;
    Temp0 ^= C;
    
    return Temp0;
}


} // /namespace math

} // /namespace sp



// ================================================================================
