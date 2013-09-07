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
#include "Framework/Tools/ScriptParser/spUtilityTokenIterator.hpp"


namespace sp
{
namespace tool
{


//! Token parser comment styles.
enum ETokenCommentStyles
{
    COMMENTSTYLE_NONE,      //!< No comments.
    COMMENTSTYLE_ANSI_C,    //!< ANSI C style comments: '//' (also multi line comments)
    COMMENTSTYLE_HTML,      //!< HTML style comments: '<!--' & '-->'
    COMMENTSTYLE_BASH,      //!< Bash script style comments: '#'
    COMMENTSTYLE_BASIC,     //!< BASIC style comments: ';'
};

//! Token parser flags.
enum ETokenParserFlags
{
    PARSERFLAG_IGNORE_WHITESPACES = 0x0001, //!< Ignores all white spaces: ' ', '\t' and '\n'.
};


/**
The token parser is used to - as the name implies - parse all tokens from a given source string.
This is the first step in compiling and/or interpreting source code. The syntax are treated as in C++
i.e. '+=', '>>=', '--' etc. are treated as a single token. The same goes for comments.
\since Version 3.2
\todo Parsing HTML files is incomplete!
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
        \param[in] CommentStyle Specifies the comment style. By default COMMENTSTYLE_NONE.
        \param[in] Flags Specifies some options parsing flags.
        This can be a combination of the ETokenParserFlags enumeration values.
        \return TokenIterator shared pointer or null if an error occured.
        \see ETokenParserFlags
        \see ETokenCommentStyles
        \see TokenIteratorPtr
        */
        TokenIteratorPtr parseTokens(
            const c8* InputString, const ETokenCommentStyles CommentStyle = COMMENTSTYLE_NONE, s32 Flags = 0
        );
        
        /**
        Does the same as the "readTokens" procedure but gets the string directly from a file.
        \param[in] Filename Specifies the input file which is to be read.
        \see readTokens
        */
        TokenIteratorPtr parseFile(
            const io::stringc &InputFilename, const ETokenCommentStyles CommentStyle = COMMENTSTYLE_NONE, s32 Flags = 0
        );
        
    private:
        
        /* === Functions === */
        
        void nextChar();
        void ignore(u32 Count);
        
        TokenIteratorPtr exitWithError(const io::stringc &Message);
        
        void addToken(const ETokenTypes TokenType);
        void addToken(const ETokenTypes TokenType, const io::stringc &TokenStr);
        void addToken(const ETokenTypes TokenType, c8 TokenChr);
        
        void parseWhiteSpace();
        
        c8 getFollowingChar(s32 Offset) const;
        
        bool isChar(c8 Chr0, c8 Chr1, c8 Chr2) const;
        bool isChar(c8 Chr0, c8 Chr1, c8 Chr2, c8 Chr3) const;
        
        void copyStringPart(io::stringc &CurrString, const c8* CurrStringStart);
        
        /* === Inline functions === */
        
        inline void ignore()
        {
            nextChar();
        }
        
        inline bool isChar(c8 Char) const
        {
            return CurrChar_ == Char;
        }
        inline bool isChar(c8 Char, c8 NextChar) const
        {
            return CurrChar_ == Char && NextChar_ == NextChar;
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
        
        s32 Flags_;
        
        std::list<SToken> OutputTokens_;
        
        const c8* InputString_;
        const c8* CurrCharPtr_;
        
        c8 CurrChar_, NextChar_;
        s32 Row_, Column_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
