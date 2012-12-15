/*
 * MD5 check sum header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_MD5_CHECKSUM_H__
#define __SP_MATH_MD5_CHECKSUM_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"


namespace sp
{
namespace math
{


/**
128 bit integer with bitwise operators. This is primary used in the MD5CheckSum class.
\see MD5CheckSum
\since Version 3.2
*/
class UInt128
{
    
    public:
        
        UInt128() :
            High_   (0),
            Low_    (0)
        {
        }
        UInt128(u64 HighWord, u64 LowWord) :
            High_   (HighWord   ),
            Low_    (LowWord    )
        {
        }
        UInt128(u32 Word3, u32 Word2, u32 Word1, u32 Word0) :
            High_   (static_cast<u64>(Word3) << 8 | Word2),
            Low_    (static_cast<u64>(Word1) << 8 | Word0)
        {
        }
        UInt128(const UInt128 &Other) :
            High_   (Other.High_),
            Low_    (Other.Low_ )
        {
        }
        ~UInt128()
        {
        }
        
        /* === Operators === */
        
        inline bool operator == (const UInt128 &Other) const
        {
            return High_ == Other.High_ && Low_ == Other.Low_;
        }
        inline bool operator != (const UInt128 &Other) const
        {
            return High_ != Other.High_ || Low_ != Other.Low_;
        }
        
        #define BITWISE_OP(x)                                   \
            inline UInt128& operator x (const UInt128 &Other)   \
            {                                                   \
                High_   x Other.High_;                          \
                Low_    x Other.Low_;                           \
                return *this;                                   \
            }
        
        BITWISE_OP(=)
        BITWISE_OP(&=)
        BITWISE_OP(|=)
        BITWISE_OP(^=)
        
        #undef BITWISE_OP
        
        /* === Inline functions === */
        
        inline UInt128& invert()
        {
            High_   = ~High_;
            Low_    = ~Low_;
            return *this;
        }
        
        inline u64 getHighWord() const
        {
            return High_;
        }
        inline u64 getLowWord() const
        {
            return Low_;
        }
        
    private:
        
        /* === Members: === */
        
        u64 High_;
        u64 Low_;
        
};


/**
MD5 check sum class. Usage example:
\code
// My message
io::stringc Message = "My message which is to be check-sum'ed";
// Create MD5 check sum object
MD5CheckSum CheckSum(Message.c_str(), Message.size());
// Get hashed code as hex string
io::stringc HexCode = CheckSum.getHexString();
\endcode
\since Version 3.2
*/
class SP_EXPORT MD5CheckSum
{
    
    public:
        
        MD5CheckSum();
        MD5CheckSum(const void* Buffer, u32 Size);
        MD5CheckSum(const MD5CheckSum &Other);
        ~MD5CheckSum();
        
        /* === Functions === */
        
        //! Returns true if this is a valid MD5 check sum. Otherwise all 128 bits are 0.
        bool valid() const;
        
        //! Returns the check-sum code as hex string (e.g. "EF07BB04FE5F05C55A77B0231F50FC6F").
        io::stringc getHexString() const;
        
        /* === Inline functions === */
        
        inline MD5CheckSum& operator = (const MD5CheckSum &Other)
        {
            CheckSum_ = Other.CheckSum_;
            return *this;
        }
        
        inline bool operator == (const MD5CheckSum &Other) const
        {
            return CheckSum_ == Other.CheckSum_;
        }
        inline bool operator != (const MD5CheckSum &Other) const
        {
            return CheckSum_ != Other.CheckSum_;
        }
        
        //! Returns the computed check sum.
        inline UInt128 get() const
        {
            return CheckSum_;
        }
        
    private:
        
        /* === Functions: === */
        
        void computeCheckSum(const void* Buffer, u32 Size);
        
        UInt128 getF(const UInt128 &B, const UInt128 &C, const UInt128 &D) const;
        UInt128 getG(const UInt128 &B, const UInt128 &C, const UInt128 &D) const;
        UInt128 getH(const UInt128 &B, const UInt128 &C, const UInt128 &D) const;
        UInt128 getI(const UInt128 &B, const UInt128 &C, const UInt128 &D) const;
        
        /* === Members: === */
        
        UInt128 CheckSum_;
        
};


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
