/*
 * Token iterator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_TOKEN_ITERATOR_H__
#define __SP_UTILITY_TOKEN_ITERATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_TOKENPARSER


#include "Base/spInputOutputString.hpp"

#include <boost/shared_ptr.hpp>


namespace sp
{
namespace tool
{


//! Script token types.
enum ETokenTypes
{
    TOKEN_UNKNOWN,                  //!< Unknown token.
    
    /* Nam and strings */
    TOKEN_NAME,                     //!< Name of a variable, function, keyword etc.
    TOKEN_STRING,                   //!< ANSI C strings.
    
    /* Numbers */
    TOKEN_NUMBER_INT,               //!< Integer numbers.
    TOKEN_NUMBER_FLOAT,             //!< Floating point numbers.
    
    /* Special signs */
    TOKEN_COMMA,                    //!< ,
    TOKEN_DOT,                      //!< .
    TOKEN_COLON,                    //!< :
    TOKEN_SEMICOLON,                //!< ;
    TOKEN_EXCLAMATION_MARK,         //!< !
    TOKEN_QUESTION_MARK,            //!< ?
    TOKEN_HASH,                     //!< #
    TOKEN_AT,                       //!< @
    TOKEN_DOLLAR,                   //!< $
    TOKEN_BRACKET_LEFT,             //!< (
    TOKEN_BRACKET_RIGHT,            //!< )
    TOKEN_SQUARED_BRACKET_LEFT,     //!< [
    TOKEN_SQUARED_BRACKET_RIGHT,    //!< ]
    TOKEN_BRACE_LEFT,               //!< {
    TOKEN_BRACE_RIGHT,              //!< }
    TOKEN_GREATER_THAN,             //!< >
    TOKEN_LESS_THAN,                //!< <
    TOKEN_EQUAL,                    //!< =
    TOKEN_ADD,                      //!< +
    TOKEN_SUB,                      //!< -
    TOKEN_MUL,                      //!< *
    TOKEN_DIV,                      //!< /
    TOKEN_MOD,                      //!< %
    TOKEN_TILDE,                    //!< ~
    TOKEN_AND,                      //!< &
    TOKEN_OR,                       //!< |
    TOKEN_XOR,                      //!< ^
    
    /* White spaces */
    TOKEN_BLANK,                    //!< ' '
    TOKEN_TAB,                      //!< '\t'
    TOKEN_NEWLINE,                  //!< '\n'
    
    /* End of file token */
    TOKEN_EOF,                      //!< Enf of file
};

/**
Validatable bracket types.
\see TokenIterator::validateBrackets
*/
enum EValidateBracketTypes
{
    VALIDATE_BRACKET            = 0x01, //!< Validates '(' and ')'.
    VALIDATE_SQUARED_BRACKET    = 0x02, //!< Validates '[' and ']'.
    VALIDATE_BRACE              = 0x04, //!< Validates '{' and '}'.
};


//! Script token structure.
struct SP_EXPORT SToken
{
    SToken();
    SToken(const SToken &Other);
    SToken(const ETokenTypes TokenType, s32 TokenRow = 0, s32 TokenColumn = 0);
    SToken(
        const ETokenTypes TokenType, const io::stringc &TokenStr,
        s32 TokenRow = 0, s32 TokenColumn = 0
    );
    SToken(
        const ETokenTypes TokenType, c8 TokenChr,
        s32 TokenRow = 0, s32 TokenColumn = 0
    );
    ~SToken();
    
    /* === Functions === */
    
    //! Returns the stored row and column of this token as a string (e.g. "[5:17]").
    io::stringc getRowColumnString() const;
    
    //! Returns true if this token is from the type TOKEN_NAME and the string matches the specified string.
    bool isName(const io::stringc &Name) const;
    //! Returns true if this token is from the type TOKEN_BLANK, TOKEN_TAB or TOKEN_NEWLINE. \see ETokenTypes
    bool isWhiteSpace() const;
    
    /* === Inline functions === */
    
    //! Returns true if this token is from the type TOKEN_EOF. \see ETokenTypes
    inline bool eof() const
    {
        return Type == TOKEN_EOF;
    }
    
    /* === Members === */
    
    ETokenTypes Type;   //!< Token type. \see ETokenTypes
    io::stringc Str;    //!< Token string. This is only used when the token type is TOKEN_NAME, TOKEN_STRING, TOKEN_NUMBER_FLOAT or TOKEN_NUMBER_INT.
    c8 Chr;             //!< Token character. This is only used when the token type is one of the special signs.
    s32 Row;            //!< Row (or rather line) in string.
    s32 Column;         //!< Column in string.
};


/**
The token iterator is used as output from the token parser.
With such an object you can iterate easier over the token container.
\since Version 3.3
*/
class SP_EXPORT TokenIterator
{
    
    public:
        
        TokenIterator(const std::list<SToken> &TokenList);
        ~TokenIterator();
        
        /* === Functions === */
        
        //! Returns the current token.
        const SToken& getToken();
        
        const SToken& getNextToken(bool IgnoreWhiteSpaces = true);
        const SToken& getPrevToken(bool IgnoreWhiteSpaces = true);
        
        const SToken& getNextToken(const ETokenTypes NextTokenType, bool IgnoreWhiteSpaces = true);
        const SToken& getPrevToken(const ETokenTypes NextTokenType, bool IgnoreWhiteSpaces = true);
        
        const SToken& getNextToken(const ETokenTypes NextTokenType, u32 &SkipedTokens, bool IgnoreWhiteSpaces = true);
        const SToken& getPrevToken(const ETokenTypes NextTokenType, u32 &SkipedTokens, bool IgnoreWhiteSpaces = true);
        
        bool next();
        bool prev();
        
        /**
        Validates the brackets, i.e. checks if all opening brackets ('(', '[', '{')
        follow the respective closing bracket (')', ']', '}').
        \param[in] Flags Specifies which types or brackets are to be validated.
        This can be a combination of the "EValidateBracketTypes" enumeration types.
        By default all bracket types will be validated.
        \return Null pointer on success. Otherwise the returned constant pointer points to the invalid token.
        \see EValidateBracketTypes
        */
        const SToken* validateBrackets(s32 Flags = ~0) const;
        
    private:
        
        /* === Members === */
        
        std::vector<SToken> Tokens_;
        u32 Index_;
        
        static SToken InvalidToken_;
        
};

typedef boost::shared_ptr<TokenIterator> TokenIteratorPtr;


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
