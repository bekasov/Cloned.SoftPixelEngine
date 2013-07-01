/*
 * Token parser file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityTokenParser.hpp"

#ifdef SP_COMPILE_WITH_TOKENPARSER


#include "Base/spInputOutputFileSystem.hpp"

#include <boost/make_shared.hpp>


namespace sp
{
namespace tool
{


TokenParser::TokenParser() :
    Flags_      (0),
    CurrChar_   (0),
    NextChar_   (0),
    Row_        (0),
    Column_     (0)
{
}
TokenParser::~TokenParser()
{
}

TokenIteratorPtr TokenParser::parseTokens(
    const c8* InputString, const ETokenCommentStyles CommentStyle, s32 Flags)
{
    /* Internal macros */
    #define PARSE_TOKEN(c, n)       \
        if (isChar(c))              \
        {                           \
            addToken(TOKEN_##n, c); \
            continue;               \
        }
    
    /* Temporary memory */
    if (!InputString)
        return TokenIteratorPtr();
    
    OutputTokens_.clear();
    InputString_ = InputString;
    
    bool IsCommentLine      = false;
    bool IsCommentMultiLine = false;
    bool IsString           = false;
    bool IsName             = false;
    bool IsNumber           = false;
    bool HasNumberDot       = false;
    
    io::stringc CurrString;
    
    Flags_  = Flags;
    Row_    = 1;
    Column_ = 1;
    
    /* Get first character */
    NextChar_ = *InputString_;
    
    /* Read each character in string */
    do
    {
        /* Get next character */
        nextChar();
        
        /* Check for active comments */
        if (CommentStyle != COMMENTSTYLE_NONE)
        {
            if (IsCommentLine)
            {
                if (isChar('\n'))
                    IsCommentLine = false;
                continue;
            }
            if (IsCommentMultiLine)
            {
                if (CommentStyle == COMMENTSTYLE_ANSI_C && isChar('*', '/'))
                {
                    IsCommentMultiLine = false;
                    ignore();
                }
                else if (CommentStyle == COMMENTSTYLE_HTML && isChar('-', '-', '>'))
                {
                    IsCommentMultiLine = false;
                    ignore(2);
                }
                continue;
            }
        }
        
        /* Check for active string */
        if (IsString)
        {
            if (isChar('\\'))
            {
                /* Get next character */
                nextChar();
                
                /* Add special character to string */
                switch (CurrChar_)
                {
                    case 't':   CurrString += '\t'; break;
                    case 'n':   CurrString += '\n'; break;
                    case '\"':  CurrString += '\"'; break;
                    default:
                        return exitWithError("Incomplete character after '\\' character");
                }
            }
            else if (isChar('\"'))
            {
                /* Add string token */
                IsString = false;
                addToken(TOKEN_STRING, CurrString);
            }
            else
            {
                /* Add current character to string */
                CurrString += CurrChar_;
            }
            continue;
        }
        else if (isChar('\"'))
        {
            /* Start with string reading */
            CurrString  = "";
            IsString    = true;
            continue;
        }
        
        /* Check for starting comments */
        switch (CommentStyle)
        {
            case COMMENTSTYLE_ANSI_C:
                if (isChar('/', '/'))
                {
                    IsCommentLine = true;
                    ignore();
                    continue;
                }
                if (isChar('/', '*'))
                {
                    IsCommentMultiLine = true;
                    ignore();
                    continue;
                }
                break;
                
            case COMMENTSTYLE_HTML:
                if (isChar('<', '!', '-', '-'))
                {
                    IsCommentMultiLine = true;
                    ignore(3);
                    continue;
                }
                break;
                
            case COMMENTSTYLE_BASH:
                if (isChar('#'))
                {
                    IsCommentLine = true;
                    continue;
                }
                break;
                
            case COMMENTSTYLE_BASIC:
                if (isChar(';'))
                {
                    IsCommentLine = true;
                    continue;
                }
                break;
                
            default:
                break;
        }
        
        /* Check for white spaces */
        if (isCharWhiteSpace(CurrChar_))
        {
            parseWhiteSpace();
            continue;
        }
        
        /* Check for names */
        if (!IsName && isCharNamePart(CurrChar_))
        {
            /* Start with name reading */
            CurrString  = "";
            IsName      = true;
        }
        
        if (IsName)
        {
            CurrString += CurrChar_;
            
            if (!isCharNamePart(NextChar_) && !isCharNumber(NextChar_))
            {
                /* Add name token */
                IsName = false;
                addToken(TOKEN_NAME, CurrString);
            }
            
            continue;
        }
        
        /* Check for numbers */
        if ( !IsNumber && ( isCharNumber(CurrChar_) || ( isChar('.') && isCharNumber(NextChar_) ) ) )
        {
            /* Start with number reading */
            CurrString      = "";
            IsNumber        = true;
            HasNumberDot    = false;
        }
        
        if (IsNumber)
        {
            CurrString += CurrChar_;
            
            if (isChar('.'))
            {
                if (HasNumberDot)
                    return exitWithError("Too many dots in number");
                else
                    HasNumberDot = true;
                
                if (!isCharNumber(NextChar_))
                    return exitWithError("Floating point number without a number after the dot");
            }
            else if (!isCharNumber(NextChar_) && isNotNextChar('.'))
            {
                /* Add number token */
                IsNumber = false;
                if (HasNumberDot)
                    addToken(TOKEN_NUMBER_FLOAT, CurrString);
                else
                    addToken(TOKEN_NUMBER_INT, CurrString);
            }
            
            continue;
        }
        
        /* Check for special signs */
        PARSE_TOKEN(',', COMMA                  )
        PARSE_TOKEN('.', DOT                    )
        PARSE_TOKEN(':', COLON                  )
        PARSE_TOKEN(';', SEMICOLON              )
        PARSE_TOKEN('!', EXCLAMATION_MARK       )
        PARSE_TOKEN('?', QUESTION_MARK          )
        PARSE_TOKEN('#', HASH                   )
        PARSE_TOKEN('@', AT                     )
        PARSE_TOKEN('$', DOLLAR                 )
        PARSE_TOKEN('(', BRACKET_LEFT           )
        PARSE_TOKEN(')', BRACKET_RIGHT          )
        PARSE_TOKEN('[', SQUARED_BRACKET_LEFT   )
        PARSE_TOKEN(']', SQUARED_BRACKET_RIGHT  )
        PARSE_TOKEN('{', BRACE_LEFT             )
        PARSE_TOKEN('}', BRACE_RIGHT            )
        PARSE_TOKEN('>', GREATER_THAN           )
        PARSE_TOKEN('<', LESS_THAN              )
        PARSE_TOKEN('=', EQUAL                  )
        PARSE_TOKEN('+', ADD                    )
        PARSE_TOKEN('-', SUB                    )
        PARSE_TOKEN('*', MUL                    )
        PARSE_TOKEN('/', DIV                    )
        PARSE_TOKEN('%', MOD                    )
        PARSE_TOKEN('~', TILDE                  )
        PARSE_TOKEN('&', AND                    )
        PARSE_TOKEN('|', OR                     )
        PARSE_TOKEN('^', XOR                    )
    }
    while (*InputString_ != 0);
    
    addToken(TOKEN_EOF);
    
    /* Allocate output token iterator */
    TokenIteratorPtr OutTokenIterator = boost::make_shared<TokenIterator>(OutputTokens_);
    
    OutputTokens_.clear();
    
    return OutTokenIterator;
    
    /* Undefine internal macros */
    #undef PARSE_TOKEN
}

TokenIteratorPtr TokenParser::parseFile(
    const io::stringc &Filename, const ETokenCommentStyles CommentStyle, s32 Flags)
{
    /* Read file into string */
    io::FileSystem FileSys;
    const io::stringc Str(FileSys.readFileString(Filename));
    
    /* Prase all tokens from the string */
    return parseTokens(Str.c_str(), CommentStyle, Flags);
}


/*
 * ======= Private: =======
 */

void TokenParser::nextChar()
{
    /* Get next and current characters */
    CurrChar_ = NextChar_;
    
    if (*InputString_ != 0)
    {
        ++InputString_;
        NextChar_ = *InputString_;
    }
    else
        NextChar_ = 0;
    
    /* Increment row- and column counter */
    ++Column_;
    if (CurrChar_ == '\n')
    {
        Column_ = 1;
        ++Row_;
    }
}

void TokenParser::ignore(u32 Count)
{
    for (u32 i = 0; i < Count; ++i)
        nextChar();
}

TokenIteratorPtr TokenParser::exitWithError(const io::stringc &Message)
{
    OutputTokens_.clear();
    
    io::Log::message(
        "Token reader error [" + io::stringc(Row_) + ":" + io::stringc(Column_) + "]: " + Message + "!", io::LOG_ERROR
    );
    
    return TokenIteratorPtr();
}

void TokenParser::addToken(const ETokenTypes TokenType)
{
    OutputTokens_.push_back(SToken(TokenType, Row_, Column_));
}
void TokenParser::addToken(const ETokenTypes TokenType, const io::stringc &TokenStr)
{
    OutputTokens_.push_back(SToken(TokenType, TokenStr, Row_, Column_));
}
void TokenParser::addToken(const ETokenTypes TokenType, c8 TokenChr)
{
    OutputTokens_.push_back(SToken(TokenType, TokenChr, Row_, Column_));
}

void TokenParser::parseWhiteSpace()
{
    if ((Flags_ & PARSERFLAG_IGNORE_WHITESPACES) == 0)
    {
        switch (CurrChar_)
        {
            case ' ':
                addToken(TOKEN_BLANK, ' ');
                break;
            case '\t':
                addToken(TOKEN_TAB, '\t');
                break;
            case '\n':
                addToken(TOKEN_NEWLINE, '\n');
                break;
            default:
                break;
        }
    }
}

c8 TokenParser::getFollowingChar(s32 Offset) const
{
    return (InputString_ != 0 && InputString_[Offset - 1] != 0) ? InputString_[Offset] : 0;
}

bool TokenParser::isChar(c8 Chr0, c8 Chr1, c8 Chr2) const
{
    return isChar(Chr0, Chr1) && getFollowingChar(1) == Chr2;
}

bool TokenParser::isChar(c8 Chr0, c8 Chr1, c8 Chr2, c8 Chr3) const
{
    return isChar(Chr0, Chr1) && getFollowingChar(1) == Chr2 && getFollowingChar(2) == Chr3;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
