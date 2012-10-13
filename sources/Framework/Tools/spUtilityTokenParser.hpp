/*
 * Token parser header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_TOKEN_READER_H__
#define __SP_UTILITY_TOKEN_READER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TOKENPARSER


#include "Base/spInputOutputString.hpp"


namespace sp
{
namespace tool
{


//! Script token types.
enum ETokenTypes
{
    TOKEN_UNKNOWN,                  //!< Unknown token.
    TOKEN_NAME,                     //!< Name of a variable, function, keyword etc.
    TOKEN_STRING,                   //!< ANSI C strings.
    TOKEN_NUMBER_INT,               //!< Integer numbers.
    TOKEN_NUMBER_FLOAT,             //!< Floating point numbers.
    TOKEN_COMMA,                    //!< ,
    TOKEN_DOT,                      //!< .
    TOKEN_COLON,                    //!< :
    TOKEN_SEMICOLON,                //!< ;
    TOKEN_EXCLAMATION_MARK,         //!< !
    TOKEN_QUESTION_MARK,            //!< ?
    TOKEN_BRACKET_LEFT,             //!< (
    TOKEN_BRACKET_RIGHT,            //!< )
    TOKEN_SQUARED_BRACKET_LEFT,     //!< [
    TOKEN_SQUARED_BRACKET_RIGHT,    //!< ]
    TOKEN_BRACE_LEFT,               //!< {
    TOKEN_BRACE_RIGHT,              //!< }
    TOKEN_GREATER_THAN,             //!< >
    TOKEN_LESS_THAN,                //!< <
    TOKEN_GREATER_THAN_OR_EQUAL,    //!< >=
    TOKEN_LESS_THAN_OR_RQUAL,       //!< <=
    TOKEN_EQUAL,                    //!< ==
    TOKEN_ASSIGN,                   //!< =
    TOKEN_NOT_EQUAL,                //!< !=
    TOKEN_INC,                      //!< ++
    TOKEN_DEC,                      //!< --
    TOKEN_ADD,                      //!< +
    TOKEN_SUB,                      //!< -
    TOKEN_MUL,                      //!< *
    TOKEN_DIV,                      //!< /
    TOKEN_MOD,                      //!< %
    TOKEN_ADD_ASSIGN,               //!< +=
    TOKEN_SUB_ASSIGN,               //!< -=
    TOKEN_MUL_ASSIGN,               //!< *=
    TOKEN_DIV_ASSIGN,               //!< /=
    TOKEN_MOD_ASSIGN,               //!< %=
    TOKEN_LOGIC_AND,                //!< &&
    TOKEN_LOGIC_OR,                 //!< ||
    TOKEN_BITWISE_NOT,              //!< ~
    TOKEN_BITWISE_AND,              //!< &
    TOKEN_BITWISE_OR,               //!< |
    TOKEN_BITWISE_XOR,              //!< ^
    TOKEN_BITWISE_NOT_ASSIGN,       //!< ~=
    TOKEN_BITWISE_AND_ASSIGN,       //!< &=
    TOKEN_BITWISE_OR_ASSIGN,        //!< |=
    TOKEN_BITWISE_XOR_ASSIGN,       //!< ^=
    TOKEN_SHIFT_LEFT,               //!< <<
    TOKEN_SHIFT_RIGHT,              //!< >>
    TOKEN_SHIFT_LEFT_ASSIGN,        //!< <<=
    TOKEN_SHIFT_RIGHT_ASSIGN,       //!< >>=
};


//! Script token structure.
struct SP_EXPORT SToken
{
    SToken();
    SToken(const ETokenTypes TokenType, s32 TokenRow = 0, s32 TokenColumn = 0);
    SToken(
        const ETokenTypes TokenType, const io::stringc &TokenStr,
        s32 TokenRow = 0, s32 TokenColumn = 0
    );
    ~SToken();
    
    /* Functions */
    io::stringc getRowColumnString() const;
    bool isName(const io::stringc &Name) const;
    
    /* Members */
    ETokenTypes Type;   //!< Token type. \see ETokenTypes
    io::stringc Str;    //!< Token string. This is only used when the token type is TOKEN_NAME or TOKEN_STRING.
    s32 Row;            //!< Row (or rather line) in string.
    s32 Column;         //!< Column in string.
};


/**
The token parser is used to - as the name implies - parse all tokens from a given source string.
This is the first step in compiling and/or interpreting source code. The syntax are treated as in C++
i.e. '+=', '>>=', '--' etc. are treated as a single token. The same goes for comments.
\since Version 3.2
*/
class SP_EXPORT TokenParser
{
    
    public:
        
        TokenParser();
        ~TokenParser();
        
        /* === Functions === */
        
        /**
        Reads all tokens out of the given string. This is not programming language specific.
        It can be used for a custom language parser as well.
        \param[in] InputString Specifies the input string. This is a null-terminated ANSI C string.
        \param[out] OutputTokens Specifies the output token container.
        \return True if any tokens could read. Otherwise false.
        \note Strings and comments will be treated as in C++.
        */
        bool readTokens(const c8* InputString, std::vector<SToken> &OutputTokens);
        
        /**
        Does the same as the "readTokens" procedure but gets the string directly from a file.
        \param[in] Filename Specifies the input file which is to be read.
        \param[out] OutputTokens Specifies the output token container.
        \return True if any tokens could read. Otherwise false.
        */
        bool readFile(const io::stringc &InputFilename, std::vector<SToken> &OutputTokens);
        
    private:
        
        /* === Functions === */
        
        void nextChar();
        
        bool exitWithError(const io::stringc &Message);
        
        void addToken(const ETokenTypes TokenType);
        void addToken(const ETokenTypes TokenType, const io::stringc &TokenStr);
        
        c8 getNextNextChar() const;
        
        /* === Inline functions === */
        
        inline bool isChar(c8 Char) const
        {
            return CurrChar_ == Char;
        }
        inline bool isChar(c8 Char, c8 NextChar) const
        {
            return CurrChar_ == Char && NextChar_ == NextChar;
        }
        inline bool isChar(c8 Char, c8 NextChar, c8 NextNextChar) const
        {
            return CurrChar_ == Char && NextChar_ == NextChar && getNextNextChar() == NextNextChar;
        }
        
        inline bool isNotNextChar(c8 Char) const
        {
            return NextChar_ != Char;
        }
        
        inline bool isCharWhiteSpace(c8 Char) const
        {
            return Char == ' ' || Char == '\n' || Char == '\t';
        }
        inline bool isCharNamePart(c8 Char) const
        {
            return (Char >= 'A' && Char <= 'Z') || (Char >= 'a' && Char <= 'z') || Char == '_';
        }
        inline bool isCharNumber(c8 Char) const
        {
            return Char >= '0' && Char <= '9';
        }
        
        /* === Members === */
        
        std::vector<SToken>* OutputTokensRef_;
        
        const c8* InputString_;
        c8 CurrChar_, NextChar_;
        s32 Row_, Column_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
