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
#include <stack>


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

/**
Token validation error types.
\see TokenIterator::validateBrackets
*/
enum ETokenValidationErrors
{
    VALIDATION_ERROR_NONE = 0,      //!< No error during bracket validation.
    VALIDATION_ERROR_UNEXPECTED,    //!< Unexpected bracket token, e.g. when you open with '(' and close with '}'.
    VALIDATION_ERROR_UNCLOSED,      //!< Unclosed bracket token, e.g. "a + ( b * c" without the closing ')'.
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
    
    /**
    Appends this token as string to the specified output string,
    e.g. if this token is a blank, the respective string will be returned, i.e. " ", "\t" or "\n".
    \since Version 3.3
    */
    void appendString(io::stringc &Str) const;
    
    /**
    Returns the token as string.
    \see appendString
    \since Version 3.3
    */
    io::stringc str() const;
    
    /* === Inline functions === */
    
    //! Returns true if this token is from the type TOKEN_EOF. \see ETokenTypes
    inline bool eof() const
    {
        return Type == TOKEN_EOF;
    }
    //! Returns true if this token is not from the type TOKEN_UNKNOWN. \see ETokenTypes
    inline bool valid() const
    {
        return Type != TOKEN_UNKNOWN;
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
        SToken& getToken();
        
        SToken& getNextToken(bool IgnoreWhiteSpaces = true, bool RestoreIterator = false);
        SToken& getPrevToken(bool IgnoreWhiteSpaces = true, bool RestoreIterator = false);
        
        SToken& getNextToken(const ETokenTypes NextTokenType, bool IgnoreWhiteSpaces = true, bool RestoreIterator = false);
        SToken& getPrevToken(const ETokenTypes NextTokenType, bool IgnoreWhiteSpaces = true, bool RestoreIterator = false);
        
        SToken& getNextToken(const ETokenTypes NextTokenType, u32 &SkipedTokens, bool IgnoreWhiteSpaces = true, bool RestoreIterator = false);
        SToken& getPrevToken(const ETokenTypes NextTokenType, u32 &SkipedTokens, bool IgnoreWhiteSpaces = true, bool RestoreIterator = false);
        
        bool next();
        bool prev();
        
        void push();
        void pop();
        
        /**
        Validates the brackets, i.e. checks if all opening brackets ('(', '[', '{')
        follow the respective closing bracket (')', ']', '}').
        \param[out] InvalidToken Pointer to the (possible) invalid token.
        \param[in] Flags Specifies which types or brackets are to be validated.
        This can be a combination of the "EValidateBracketTypes" enumeration types.
        By default all bracket types will be validated.
        \return One of the values defined in the "ETokenValidationErrors" enumeration.
        \see EValidateBracketTypes
        \see ETokenValidationErrors
        */
        ETokenValidationErrors validateBrackets(const SToken* &InvalidToken, s32 Flags = ~0) const;
        
        /**
        Validates the brackets without the invalid token output.
        \see EValidateBracketTypes
        \see ETokenValidationErrors
        */
        inline ETokenValidationErrors validateBrackets(s32 Flags = ~0) const
        {
            const SToken* Unused = 0;
            return validateBrackets(Unused, Flags);
        }
        
    private:
        
        /* === Members === */
        
        std::vector<SToken> Tokens_;
        u32 Index_;
        
        std::stack<u32> Stack_;
        
        static SToken InvalidToken_;
        
};

typedef boost::shared_ptr<TokenIterator> TokenIteratorPtr;


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
