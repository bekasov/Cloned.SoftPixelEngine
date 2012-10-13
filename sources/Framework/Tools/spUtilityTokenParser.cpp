/*
 * Token parser file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityTokenParser.hpp"

#ifdef SP_COMPILE_WITH_TOKENPARSER


#include "Base/spInputOutputFileSystem.hpp"


namespace sp
{
namespace tool
{


TokenParser::TokenParser() :
    OutputTokensRef_(0),
    CurrChar_       (0),
    NextChar_       (0),
    Row_            (0),
    Column_         (0)
{
}
TokenParser::~TokenParser()
{
}

bool TokenParser::readTokens(const c8* InputString, std::vector<SToken> &OutputTokens)
{
    /* Internal macros */
    #define CHECK_CHAR1_TOKEN(c1, n)    \
        if (isChar(c1))                 \
        {                               \
            addToken(TOKEN_##n);        \
            continue;                   \
        }
    
    #define CHECK_CHAR2_TOKEN(c1, c2, n)    \
        if (isChar(c1, c2))                 \
        {                                   \
            addToken(TOKEN_##n);            \
            nextChar();                     \
            continue;                       \
        }
    
    #define CHECK_CHAR3_TOKEN(c1, c2, c3, n)    \
        if (isChar(c1, c2, c3))                 \
        {                                       \
            addToken(TOKEN_##n);                \
            nextChar();                         \
            nextChar();                         \
            continue;                           \
        }
    
    /* Temporary memory */
    if (!InputString)
        return false;
    
    OutputTokensRef_    = &OutputTokens;
    InputString_        = InputString;
    
    bool IsCommentLine      = false;
    bool IsCommentMultiLine = false;
    bool IsString           = false;
    bool IsName             = false;
    bool IsNumber           = false;
    bool HasNumberDot       = false;
    
    io::stringc CurrString;
    
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
        if (IsCommentLine)
        {
            if (isChar('\n'))
                IsCommentLine = false;
            continue;
        }
        if (IsCommentMultiLine)
        {
            if (isChar('*', '/'))
            {
                IsCommentMultiLine = false;
                nextChar();
            }
            continue;
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
        if (isChar('/', '/'))
        {
            IsCommentLine = true;
            continue;
        }
        if (isChar('/', '*'))
        {
            IsCommentMultiLine = true;
            continue;
        }
        
        /* Check for white spaces */
        if (isCharWhiteSpace(CurrChar_))
            continue;
        
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
        
        /* Check for special characters */
        CHECK_CHAR3_TOKEN('<', '<', '=', SHIFT_LEFT_ASSIGN  )
        CHECK_CHAR3_TOKEN('>', '>', '=', SHIFT_RIGHT_ASSIGN )
        
        CHECK_CHAR2_TOKEN('<', '<', SHIFT_LEFT              )
        CHECK_CHAR2_TOKEN('>', '>', SHIFT_RIGHT             )
        CHECK_CHAR2_TOKEN('~', '=', BITWISE_NOT_ASSIGN      )
        CHECK_CHAR2_TOKEN('&', '=', BITWISE_AND_ASSIGN      )
        CHECK_CHAR2_TOKEN('|', '=', BITWISE_OR_ASSIGN       )
        CHECK_CHAR2_TOKEN('^', '=', BITWISE_XOR_ASSIGN      )
        CHECK_CHAR2_TOKEN('+', '=', ADD_ASSIGN              )
        CHECK_CHAR2_TOKEN('-', '=', SUB_ASSIGN              )
        CHECK_CHAR2_TOKEN('*', '=', MUL_ASSIGN              )
        CHECK_CHAR2_TOKEN('/', '=', DIV_ASSIGN              )
        CHECK_CHAR2_TOKEN('%', '=', MOD_ASSIGN              )
        CHECK_CHAR2_TOKEN('&', '&', LOGIC_AND               )
        CHECK_CHAR2_TOKEN('|', '|', LOGIC_OR                )
        CHECK_CHAR2_TOKEN('>', '=', GREATER_THAN_OR_EQUAL   )
        CHECK_CHAR2_TOKEN('<', '=', LESS_THAN_OR_RQUAL      )
        CHECK_CHAR2_TOKEN(':', '=', ASSIGN                  )
        CHECK_CHAR2_TOKEN('!', '=', NOT_EQUAL               )
        CHECK_CHAR2_TOKEN('+', '+', INC                     )
        CHECK_CHAR2_TOKEN('-', '-', DEC                     )
        
        CHECK_CHAR1_TOKEN(',', COMMA                )
        CHECK_CHAR1_TOKEN('.', DOT                  )
        CHECK_CHAR1_TOKEN(':', COLON                )
        CHECK_CHAR1_TOKEN(';', SEMICOLON            )
        CHECK_CHAR1_TOKEN('!', EXCLAMATION_MARK     )
        CHECK_CHAR1_TOKEN('?', QUESTION_MARK        )
        CHECK_CHAR1_TOKEN('(', BRACKET_LEFT         )
        CHECK_CHAR1_TOKEN(')', BRACKET_RIGHT        )
        CHECK_CHAR1_TOKEN('[', SQUARED_BRACKET_LEFT )
        CHECK_CHAR1_TOKEN(']', SQUARED_BRACKET_RIGHT)
        CHECK_CHAR1_TOKEN('{', BRACE_LEFT           )
        CHECK_CHAR1_TOKEN('}', BRACE_RIGHT          )
        CHECK_CHAR1_TOKEN('>', GREATER_THAN         )
        CHECK_CHAR1_TOKEN('<', LESS_THAN            )
        CHECK_CHAR1_TOKEN('=', EQUAL                )
        CHECK_CHAR1_TOKEN('+', ADD                  )
        CHECK_CHAR1_TOKEN('-', SUB                  )
        CHECK_CHAR1_TOKEN('*', MUL                  )
        CHECK_CHAR1_TOKEN('/', DIV                  )
        CHECK_CHAR1_TOKEN('%', MOD                  )
        CHECK_CHAR1_TOKEN('~', BITWISE_NOT          )
        CHECK_CHAR1_TOKEN('&', BITWISE_AND          )
        CHECK_CHAR1_TOKEN('|', BITWISE_OR           )
        CHECK_CHAR1_TOKEN('^', BITWISE_XOR          )
    }
    while (*InputString_ != 0);
    
    OutputTokensRef_ = 0;
    
    return !OutputTokens.empty();
    
    /* Undefine internal macros */
    #undef CHECK_CHAR1_TOKEN
    #undef CHECK_CHAR2_TOKEN
    #undef CHECK_CHAR3_TOKEN
}

bool TokenParser::readFile(const io::stringc &Filename, std::vector<SToken> &OutputTokens)
{
    io::FileSystem FileSys;
    const io::stringc Str(FileSys.readFileString(Filename));
    return readTokens(Str.c_str(), OutputTokens);
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

bool TokenParser::exitWithError(const io::stringc &Message)
{
    if (OutputTokensRef_)
    {
        OutputTokensRef_->clear();
        OutputTokensRef_ = 0;
    }
    
    io::Log::message(
        "Token reader error [" + io::stringc(Row_) + ":" + io::stringc(Column_) + "]: " + Message + "!", io::LOG_ERROR
    );
    
    return false;
}

void TokenParser::addToken(const ETokenTypes TokenType)
{
    OutputTokensRef_->push_back(
        SToken(TokenType, Row_, Column_)
    );
}
void TokenParser::addToken(const ETokenTypes TokenType, const io::stringc &TokenStr)
{
    OutputTokensRef_->push_back(
        SToken(TokenType, TokenStr, Row_, Column_)
    );
}

c8 TokenParser::getNextNextChar() const
{
    return (InputString_ != 0) ? InputString_[1] : 0;
}


/*
 * SToken structure
 */

SToken::SToken() :
    Type    (TOKEN_UNKNOWN  ),
    Row     (0              ),
    Column  (0              )
{
}
SToken::SToken(const ETokenTypes TokenType, s32 TokenRow, s32 TokenColumn) :
    Type    (TokenType  ),
    Row     (TokenRow   ),
    Column  (TokenColumn)
{
}
SToken::SToken(
    const ETokenTypes TokenType, const io::stringc &TokenStr, s32 TokenRow, s32 TokenColumn) :
    Type    (TokenType  ),
    Str     (TokenStr   ),
    Row     (TokenRow   ),
    Column  (TokenColumn)
{
}
SToken::~SToken()
{
}

io::stringc SToken::getRowColumnString() const
{
    return "[" + io::stringc(Row) + ":" + io::stringc(Column) + "]";
}

bool SToken::isName(const io::stringc &Name) const
{
    return Type == TOKEN_NAME && Str == Name;
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
