/*
 * String header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_IO_STRING_H__
#define __SP_IO_STRING_H__


#include "Base/spStandard.hpp"

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>


namespace sp
{
namespace io
{


//! Basic string class. This class wraps the std::string to have better string access.
template <typename T> class string
{
    
    public:
        
        string<T>()
        {
        }
        string<T>(const string<T> &Str) :
            Str_(Str.Str_)
        {
        }
        string<T>(const std::basic_string<T> &Str) :
            Str_(Str)
        {
        }
        string<T>(const std::stringstream &Str) :
            Str_(Str.str())
        {
        }
        template <typename B> string<T>(const B Value)
        {
            std::stringstream SStr;
            SStr << Value;
            Str_ = SStr.str();
        }
        ~string<T>()
        {
        }
        
        /* === Operators === */
        
        string<T>& operator = (const string<T> &Str)
        {
            Str_ = Str.Str_;
            return *this;
        }
        
        bool operator == (const string<T> &Str) const
        {
            return Str_ == Str.Str_;
        }
        bool operator != (const string<T> &Str) const
        {
            return Str_ != Str.Str_;
        }
        
        string<T> operator + (const string<T> &Str) const
        {
            return Str_ + Str.Str_;
        }
        string<T>& operator += (const string<T> &Str)
        {
            Str_ += Str.Str_;
            return *this;
        }
        
        inline T& operator [] (u32 Index)
        {
            return Str_[Index];
        }
        
        inline const T& operator [] (u32 Index) const
        {
            return Str_[Index];
        }
        
        /* === Extra functions === */
        
        //! Returns a reference to the internal std::string object.
        inline std::basic_string<T>& str()
        {
            return Str_;
        }
        //! Returns the internal std::string object.
        inline const std::basic_string<T>& str() const
        {
            return Str_;
        }
        //! Returns the null terminated ANSI C string.
        inline const T* c_str() const
        {
            return Str_.c_str();
        }
        //! Returns the string length. This is a synonym for the "length" function.
        inline u32 size() const
        {
            return Str_.size();
        }
        //! Returns the string length.
        inline u32 length() const
        {
            return Str_.length();
        }
        
        inline void resize(u32 Size)
        {
            Str_.resize(Size);
        }
        inline void clear()
        {
            Str_.clear();
        }
        
        inline bool empty() const
        {
            return Str_.empty();
        }
        
        template <typename B> B val() const
        {
            B Value = B(0);
            std::istringstream sstr(Str_);
            sstr >> Value;
            return Value;
        }
        
        string<T> left(u32 Len) const
        {
            if (Len > size())
                return *this;
            return string<T>(Str_.substr(0, Len));
        }
        
        string<T> right(u32 Len) const
        {
            if (Len > size())
                return *this;
            return string<T>(Str_.substr(Str_.size() - Len, Len));
        }
        
        string<T> mid(u32 Pos, u32 Len) const
        {
            if (Pos > size())
                return "";
            if (Pos + Len > size())
                Len = size() - Pos;
            return string<T>(Str_.substr(Pos, Len));
        }
        
        string<T> section(u32 Pos1, u32 Pos2) const
        {
            if (Pos1 > size() || Pos2 > size() || Pos1 > Pos2)
                return "";
            return string<T>(Str_.substr(Pos1, Pos2 - Pos1));
        }
        
        //! Returns this string without any blanks or tabulators.
        string<T> trim() const
        {
            std::string NewStr = Str_;
            
            for (u32 i = 0; i < NewStr.size();)
            {
                if (NewStr[i] == ' ' || NewStr[i] == '\t')
                    NewStr = NewStr.substr(0, i) + NewStr.substr(i + 1, NewStr.size() - 1);
                else
                    ++i;
            }
            
            return string<T>(NewStr);
        }
        
        //! Trims only the left side.
        string<T> ltrim() const
        {
            u32 i = 0;
            
            while ( i < Str_.size() && ( Str_[i] == ' ' || Str_[i] == '\t' ) )
                ++i;
            
            return string<T>(Str_.substr(i, Str_.size() - i));
        }
        
        //! Trims only the right side.
        string<T> rtrim() const
        {
            s32 i = static_cast<s32>(Str_.size()) - 1;
            
            while ( i >= 0 && ( Str_[i] == ' ' || Str_[i] == '\t' ) )
                --i;
            
            return string<T>(Str_.substr(0, static_cast<u32>(i + 1)));
        }
        
        //! Returns true if the first 'Len' characters of this string and the given string are equal.
        bool leftEqual(const string<T> &Str, u32 Len) const
        {
            if (Len > size())
                Len = size();
            if (Len > Str.size())
                Len = Str.size();
            
            for (u32 i = 0; i < Len; ++i)
            {
                if (Str_[i] != Str.Str_[i])
                    return false;
            }
            
            return true;
        }
        
        //! Returns true if the last 'Len' characters of this string and the given string are equal.
        bool rightEqual(const string<T> &Str, u32 Len) const
        {
            if (Len > size())
                Len = size();
            if (Len > Str.size())
                Len = Str.size();
            
            const u32 LenA = Str_.size();
            const u32 LenB = Str.Str_.size();
            
            for (u32 i = 0; i < Len; ++i)
            {
                if (Str_[LenA - i - 1] != Str.Str_[LenB - i - 1])
                    return false;
            }
            
            return true;
        }
        
        //! Returns the count of the first equal characters between this string and the given string.
        u32 getLeftEquality(const string<T> &Str) const
        {
            u32 i = 0;
            
            for (u32 c = std::min(Str_.size(), Str.Str_.size()); i < c; ++i)
            {
                if (Str_[i] != Str.Str_[i])
                    break;
            }
            
            return i;
        }
        
        //! Returns the count of the last equal characters between this string and the given string.
        u32 getRightEquality(const string<T> &Str) const
        {
            u32 i = 0;
            
            const u32 LenA = Str_.size();
            const u32 LenB = Str.Str_.size();
            
            for (u32 c = std::min(Str_.size(), Str.Str_.size()); i < c; ++i)
            {
                if (Str_[LenA - i - 1] != Str.Str_[LenB - i - 1])
                    break;
            }
            
            return i;
        }
        
        /**
        Returns the position in the string (beginngin with 0) where the searched string has been found.
        If the string could not be found the return value is -1.
        \param Str: Searched string.
        \param PosBegin: Position where the search shall begin.
        */
        inline s32 find(const string<T> &Str, u32 PosBegin = 0) const
        {
            return static_cast<s32>(Str_.find(Str.Str_, PosBegin));
        }
        
        inline s32 rfind(const string<T> &Str, u32 PosBegin = -1) const
        {
            return static_cast<s32>(Str_.rfind(Str.Str_, PosBegin));
        }
        
        s32 findChar(const T &SearchChar, u32 PosBegin = 0) const
        {
            for (u32 i = PosBegin, c = Str_.size(); i < c; ++i)
            {
                if (Str_[i] == SearchChar)
                    return i;
            }
            return -1;
        }
        
        s32 rfindChar(const T &SearchChar, u32 PosBegin = -1) const
        {
            if (PosBegin > 0)
            {
                for (s32 c = static_cast<s32>(Str_.size()), i = (PosBegin < c ? PosBegin : c); i >= 0; --i)
                {
                    if (Str_[i] == SearchChar)
                        return i;
                }
            }
            return -1;
        }
        
        /**
        Returns the position in the string (beginning with 0) where any other charaters has been found
        which are not named in the given string.
        \param Str: String with all characters which are not wanted.
        \param PosBegin: Position where the search shall begin.
        */
        s32 findNot(const string<T> &Str, u32 PosBegin = 0) const
        {
            u32 i = PosBegin, j;
            
            while (i < Str_.size())
            {
                for (j = 0; j < Str.size(); ++j)
                {
                    if (Str_[i] != Str[i])
                        return i;
                }
                ++i;
            }
            
            return -1;
        }
        
        /**
        Similiar to the "findNot" function but this one accepts each character besides spaces (' ' and '\t').
        \param PosBegin: Position where the search shall begin.
        */
        s32 findNotSpaces(u32 PosBegin = 0) const
        {
            u32 i = PosBegin;
            
            while (i < Str_.size())
            {
                if (Str_[i] != ' ' && Str_[i] != '\t')
                    return i;
                ++i;
            }
            
            return -1;
        }
        
        //! Returns the string with upper case only
        string<T> upper() const
        {
            std::string NewStr = Str_;
            
            for (u32 i = 0; i < NewStr.size(); ++i)
            {
                if (NewStr[i] >= 97 && NewStr[i] <= 122)
                    NewStr[i] -= 32;
            }
            
            return string<T>(NewStr);
        }
        
        //! Returns the string with lower case only
        string<T> lower() const
        {
            std::string NewStr = Str_;
            
            for (u32 i = 0; i < NewStr.size(); ++i)
            {
                if (NewStr[i] >= 65 && NewStr[i] <= 90)
                    NewStr[i] += 32;
            }
            
            return string<T>(NewStr);
        }
        
        //! Changes this string to upper case.
        string<T>& makeUpper()
        {
            for (u32 i = 0, c = Str_.size(); i < c; ++i)
            {
                if (Str_[i] >= 97 && Str_[i] <= 122)
                    Str_[i] -= 32;
            }
            return *this;
        }
        
        //! Changes this string to lower case.
        string<T>& makeLower()
        {
            for (u32 i = 0, c = Str_.size(); i < c; ++i)
            {
                if (Str_[i] >= 65 && Str_[i] <= 90)
                    Str_[i] += 32;
            }
            return *this;
        }
        
        string<T> replace(const string<T> &StrFind, const string<T> &StrReplace, u32 PosBegin = 0) const
        {
            std::string NewStr = Str_;
            s32 Pos = PosBegin;
            
            while ( ( Pos = NewStr.find(StrFind.str(), Pos) ) != -1)
            {
                NewStr =
                    NewStr.substr(0, Pos) +
                    StrReplace.str() +
                    NewStr.substr( Pos + StrFind.size(), NewStr.size() - (Pos + StrFind.size()) );
                Pos += StrReplace.size();
            }
            
            return string<T>(NewStr);
        }
        
        /* === Static functions === */
        
        static string<T> ascii(u8 Character)
        {
            return string<T>(std::string(1, Character));
        }
        
        static string<T> space(u32 Count, c8 Ascii = ' ')
        {
            return string<T>(std::string(Count, Ascii));
        }
        
        static string<T> create(const string<T> &Str, u32 Count = 1)
        {
            std::string NewStr;
            
            for (u32 i = 0; i < Count; ++i)
                NewStr += Str.Str_;
            
            return string<T>(NewStr);
        }
        
        /**
        Creates a string out of the given number.
        \code
        io::stringc::number( 5, 3); // This returns "005"
        io::stringc::number(16, 3); // This returns "016"
        \endcode
        */
        static string<T> number(u32 Number, u32 DigitsCount, const c8 Ascii = '0')
        {
            string<T> Str(Number);
            
            if (Str.size() < DigitsCount)
            {
                DigitsCount -= Str.size();
                Str = string<T>::space(DigitsCount, Ascii) + Str;
            }
            
            return Str;
        }
        
        /**
        Creates a string out of the given floating point number.
        \param[in] Number Specifies the floating point number which is to be converted to a string.
        \param[in] DecimalPlaces Specifies the number of digits after the dot.
        \param[in] isFillBlanks Specifies whether the missing digits after the dot are to be filled with nulls.
        \code
        io::stringc::numberFloat(34.1678, 0); // This returns "34"
        io::stringc::numberFloat(34.1678, 1); // This returns "34.1"
        io::stringc::numberFloat(34.1678, 2); // This returns "34.16"
        io::stringc::numberFloat(34.16, 4, true); // This returns "34.1600"
        \endcode
        */
        static string<T> numberFloat(f32 Number, u32 DecimalPlaces, bool isFillBlanks = false)
        {
            string<T> Str(Number);
            
            for (s32 i = static_cast<s32>(Str.size()) - 1, j = 0; i >= 0; --i, ++j)
            {
                if (Str[i] == '.')
                {
                    const s32 Count = (DecimalPlaces > 0 ? i + DecimalPlaces + 1 : i);
                    
                    if (isFillBlanks && static_cast<s32>(DecimalPlaces) > j)
                        return Str.left(Count) + string<T>::space(DecimalPlaces - j, '0');
                    
                    return Str.left(Count);
                }
            }
            
            if (isFillBlanks && DecimalPlaces > 0)
                return Str + "." + string<T>::space(DecimalPlaces, '0');
            
            return Str;
        }
        
        /**
        Converts the given string (which should only contain numbers) into a string with decimal place seperators.
        \param[in] Str Specifies the string which is to be modified.
        \param[in] Sep Specifies the seperator character. By defautl ','.
        \return The new modified string.
        \code
        io::stringc::numberSeperators("12345"); // This returns "12,345"
        io::stringc::numberSeperators("1234567", '.'); // This returns "1.234.567"
        \endcode
        */
        static string<T> numberSeperators(const string<T> &Str, const c8 Sep = ',')
        {
            std::basic_string<T> NewStr(Str.Str_);
            
            s32 i = 3 - (NewStr.size() % 3);
            
            for (std::basic_string<T>::iterator it = NewStr.begin(); it != NewStr.end(); ++it)
            {
                if (i == 3)
                {
                    i = 0;
                    if (it != NewStr.begin())
                    {
                        it = NewStr.insert(it, Sep);
                        --i;
                    }
                }
                ++i;
            }
            
            return string<T>(NewStr);
        }
        
        /* === Extra operation functions === */
        
        //! Returns the path part of the string (e.g. "C:/Users/Lukas/Documents/Test.txt" -> "C:/Users/Lukas/Documents/").
        string<T> getPathPart() const
        {
            for (s32 i = static_cast<s32>(Str_.size()) - 1; i >= 0; --i)
            {
                if (Str_[i] == '/' || Str_[i] == '\\')
                    return left(static_cast<u32>(i + 1));
            }
            return string<T>("");
        }
        
        //! Returns the extension part of the string (e.g. "C:/Users/Lukas/Documents/Test.txt" -> "txt").
        string<T> getExtensionPart() const
        {
            u32 j = 0;
            
            for (s32 i = static_cast<s32>(Str_.size()) - 1; i >= 0; --i, ++j)
            {
                if (Str_[i] == '.')
                    return right(j);
            }
            
            return string<T>("");
        }
        
        //! Returns the file part of the string (e.g. "C:/Users/Lukas/Documents/Test.txt" -> "Test.txt").
        string<T> getFilePart() const
        {
            u32 j = 0;
            
            for (s32 i = static_cast<s32>(Str_.size()) - 1; i >= 0; --i, ++j)
            {
                if (Str_[i] == '/' || Str_[i] == '\\')
                    return right(j);
            }
            
            return *this;
        }
        
        // Returns the strict file part of the string (e.g. "C:/Users/Lukas/Documents/Test.txt" -> "Test").
        string<T> getStrictFilePart() const
        {
            s32 i, j, c = static_cast<s32>(Str_.size()) - 1;
            
            for (i = j = c; i >= 0; --i)
            {
                if (j == c && Str_[i] == '.')
                    j = i;
                else if (Str_[i] == '/' || Str_[i] == '\\')
                    return section(static_cast<u32>(i + 1), static_cast<u32>(j));
            }
            
            return *this;
        }
        
        /**
        Removes each redundant "./" strings out of the path string but keeps each "../".
        The string needs to be a valid path (or rather directory name) or filename.
        */
        void adjustPath()
        {
            *this = replace("\\", "/");
            
            /*
             * Repalce each "../" with "?" that the next 'replace' call
             * will not remove the valid "../" but each redundant "./"
             * and make this operation undo again.
             */
            *this = replace("../", "?");
            *this = replace("./", "");
            *this = replace("?", "../");
        }
        
        string<c8> toAscii() const
        {
            string<c8> ascii;
            ascii.resize(Str_.size());
            
            for (u32 i = 0; i < Str_.size(); ++i)
                ascii[i] = static_cast<c8>(Str_[i]);
            
            return ascii;
        }
        
        string<c16> toUnicode() const
        {
            string<c16> unicode;
            unicode.resize(Str_.size());
            
            for (u32 i = 0; i < Str_.size(); ++i)
                unicode[i] = static_cast<c16>(Str_[i]);
            
            return unicode;
        }
        
    private:
        
        /* Members */
        
        std::basic_string<T> Str_;
        
};

typedef string<c8> stringc;
typedef string<c16> stringw;


/* Extendet template functions */

//! Converts the specified humber into a hex value as string (e.g. 256 -> "FF").
template <typename T> stringc getHexString(const T &Number)
{
    stringc Str;
    
    for (s32 i = 2*sizeof(T) - 1; i >= 0; --i)
        Str += "0123456789ABCDEF"[((Number >> i*4) & 0xF)];
    
    return Str;
}

template <typename T> T getHexNumber(const io::stringc &Str)
{
    T Result;
    std::stringstream SStr;
    SStr << std::hex << Str.str();
    SStr >> Result;
    return Result;
}

template <typename T> string<T> operator + (const c8* cStr, const string<T> &spStr)
{
    return string<T>(cStr) + spStr;
}

//! Stores all values separated by the ';' character in the specified string in the output list (e.g. "1;2;3;" -> { 1, 2, 3 }).
template < class T, template < class TL, class Allocator = std::allocator<TL> > class L > void getListFromString(
    const stringc &Str, L<T> &List, s32 MaxCount = -1)
{
    s32 Pos1 = 0, Pos2 = 0;
    
    /* Search for each ';' character to find all values in the vector */
    while ( ( MaxCount == -1 || MaxCount > 0 ) && Pos1 < Str.size() - 1 && ( Pos1 = Str.find(";", Pos2) ) != -1 )
    {
        List.push_back(Str.section(Pos2, Pos1).val<T>());
        Pos2 = Pos1 + 1;
        if (MaxCount > 0)
            --MaxCount;
    }
}

template <class T> void getListFromString(const stringc &Str, std::vector<T> &List, s32 MaxCount = -1)
{
    getListFromString<T, std::vector>(Str, List, MaxCount);
}

template <class T> void getListFromString(const stringc &Str, std::list<T> &List, s32 MaxCount = -1)
{
    getListFromString<T, std::list>(Str, List, MaxCount);
}


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
