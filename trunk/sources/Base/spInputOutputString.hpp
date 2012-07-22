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
            std::stringstream SStr; SStr << Value; Str_ = SStr.str();
        }
        ~string<T>()
        {
        }
        
        /* === Operators === */
        
        string<T> operator = (const string<T> &Str)
        {
            Str_ = Str.Str_; return Str_;
        }
        string<T> operator = (const std::basic_string<T> Str)
        {
            Str_ = Str; return Str_;
        }
        string<T> operator = (const std::stringstream Str)
        {
            Str_ = Str.str(); return Str_;
        }
        template <typename B> string<T> operator = (const B Value)
        {
            std::stringstream SStr; SStr << Value; Str_ = SStr.str(); return Str_;
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
        string<T> operator += (const string<T> &Str)
        {
            return Str_ += Str.Str_;
        }
        
        inline T& operator [] (s32 index)
        {
            return Str_[index];
        }
        
        inline const T& operator [] (s32 index) const
        {
            return Str_[index];
        }
        
        /* === Extra functions === */
        
        inline std::basic_string<T> str() const
        {
            return Str_;
        }
        inline const T* c_str() const
        {
            return Str_.c_str();
        }
        inline u32 size() const
        {
            return Str_.size();
        }
		
		inline void resize(u32 Size)
		{
			Str_.resize(Size);
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
        
        //! Returns this stirng without any blanks or tabulators.
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
            s32 i;
            for (i = 0; i < Str_.size() && ( Str_[i] == ' ' || Str_[i] == '\t' ); ++i)
            {
                // do nothing
            }
            
            return string<T>(Str_.substr(i, Str_.size() - i));
        }
        
        //! Trims only the right side.
        string<T> rtrim() const
        {
            s32 i;
            for (i = Str_.size() - 1; i >= 0 && ( Str_[i] == ' ' || Str_[i] == '\t' ); --i)
            {
                // do nothing
            }
            
            return string<T>(Str_.substr(0, i + 1));
        }
        
        /**
        Returns the position in the string (beginngin with 0) where the searched string has been found.
        If the string could not found the return value is -1.
        \param Str: Searched string.
        \param PosBegin: Position where the search shall begin.
        */
        s32 find(const string<T> &Str, u32 PosBegin = 0) const
        {
            return static_cast<s32>(Str_.find(Str.Str_, PosBegin));
        }
        
        s32 rfind(const string<T> &Str, u32 PosBegin = -1) const
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
                for (u32 c = Str_.size(), i = (PosBegin < c ? PosBegin : c); i >= 0; --i)
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
        
        //! Creates a string out of the given number. e.g. number(5, 3) return "005" or number(16, 3) return "016".
        static string<T> number(u32 Number, u32 DigitsCount, const c8 Ascii = '0')
        {
            io::string<T> Str(Number);
            
            if (Str.size() < DigitsCount)
            {
                DigitsCount -= Str.size();
                Str = io::string<T>::space(DigitsCount, Ascii) + Str;
            }
            
            return Str;
        }
        
        /* === Extra operation functions === */
        
        string<T> getPathPart() const
        {
            for (s32 i = Str_.size() - 1; i >= 0; --i)
            {
                if (Str_[i] == '/' || Str_[i] == '\\')
                    return left(i + 1);
            }
            return "";
        }
        
        string<T> getExtensionPart() const
        {
            for (s32 i = Str_.size() - 1, j = 0; i >= 0; --i, ++j)
            {
                if (Str_[i] == '.')
                    return right(j);
            }
            return string<T>("");
        }
        
        string<T> getFilePart() const
        {
            return right(size() - getPathPart().size());
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

typedef string<c8>  stringc;
typedef string<c16> stringw;


/* Extendet template functions */

//! Converts the specified humber into a hex value as string (e.g. 256 -> "FF").
template <typename T> stringc getHex(const T &Number)
{
    stringc Str;
    
    for (s32 i = 2*sizeof(T) - 1; i >= 0; --i)
        Str += "0123456789ABCDEF"[((Number >> i*4) & 0xF)];
    
    return Str;
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
